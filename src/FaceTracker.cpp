#include "FaceTracker.hpp"
#include "cinder/app/App.h"
#include "CinderOpenCV.h"
#include "cinder/Log.h"
#include "cinder/Filesystem.h"

using namespace ci;
using namespace ci::app;
using namespace std;

// ---------------------------------------------------------------------------
//  Setup: Sucht automatisch nach Modellen im Ordner 'assets/models'
// ---------------------------------------------------------------------------
void FaceTracker::setup()
{
    CI_LOG_I("Starte FaceTracker Setup");

    fs::path modelDir = getAssetPath("") / "models";

    if (!fs::exists(modelDir)) {
        CI_LOG_E("FEHLER: Ordner 'assets/models' nicht gefunden!");
        return;
    }

    for (const auto& entry : fs::directory_iterator(modelDir))
    {
        if (entry.is_regular_file())
        {
            string filename = entry.path().filename().string();
            string filepath = entry.path().string();

            std::transform(filename.begin(), filename.end(), filename.begin(), ::tolower);

            try {
                // "face" im Namen -> als YuNet laden
                if (filename.find("face") != string::npos && filename.find(".onnx") != string::npos) {
                    console() << "Lade Face Detector: " << filename << endl;
                    mFaceDetector = cv::FaceDetectorYN::create(filepath, "", cv::Size(320, 320), 0.6f, 0.3f, 5000);
                }

                // 2. "emotion" im Namen -> als Emotion Net laden
                if (filename.find("emotion") != string::npos && filename.find(".onnx") != string::npos) {
                    console() << "Lade Emotion Net: " << filename << endl;
                    mEmotionNet = cv::dnn::readNetFromONNX(filepath);
                }
            }
            catch (const cv::Exception& e) {
                CI_LOG_E("Fehler beim Laden von " << filename << ": " << e.what());
            }
        }
    }

    if (mFaceDetector) CI_LOG_I("Face Detector bereit.");
    else CI_LOG_E("ACHTUNG: Kein Face-Modell (mit 'face' im Namen) gefunden!");

    if (!mEmotionNet.empty()) CI_LOG_I("Emotion Net bereit.");
    else CI_LOG_E("ACHTUNG: Kein Emotion-Modell (mit 'emotion' im Namen) gefunden!");
}

// ---------------------------------------------------------------------------
//  Update: Emotionen etc
// ---------------------------------------------------------------------------
void FaceTracker::update(const ci::Surface8u& src)
{
    // --- setup fehlgeschlagen -> hier nichts tun ---
    if (mFaceDetector.empty() || mEmotionNet.empty()) return;

    try {
        cv::Mat frame = toOcv(src);
        if (frame.empty()) return;

        mSourceWidth = (float)frame.cols;
        mSourceHeight = (float)frame.rows;

        mFaceDetector->setInputSize(frame.size());
        cv::Mat faces;
        mFaceDetector->detect(frame, faces);

        if (faces.rows > 0)
        {
            mHasFace = true;
            float* faceData = faces.ptr<float>(0);
            cv::Rect2f currentBox(faceData[0], faceData[1], faceData[2], faceData[3]);

            // --- Glättung ---
            if (mLastFace.width <= 0) mLastFace = currentBox;
            else {
                mLastFace.x = glm::mix(mLastFace.x, currentBox.x, SMOOTH_FACTOR);
                mLastFace.y = glm::mix(mLastFace.y, currentBox.y, SMOOTH_FACTOR);
                mLastFace.width = glm::mix(mLastFace.width, currentBox.width, SMOOTH_FACTOR);
                mLastFace.height = glm::mix(mLastFace.height, currentBox.height, SMOOTH_FACTOR);
            }

            // --- Emotion erkennen ---
            // --- Bereich prüfen (verhindert Crash am Bildrand) ---
            cv::Rect roi = currentBox;
            // --- schneidet alles ab, was über den Rand ---
            roi = roi & cv::Rect(0, 0, frame.cols, frame.rows);

            if (roi.area() > 0 && roi.width > 0 && roi.height > 0) {
                cv::Mat faceROI = frame(roi);
                cv::Mat grayFace, resizedFace;
                if (faceROI.channels() == 3) cv::cvtColor(faceROI, grayFace, cv::COLOR_BGR2GRAY);
                else grayFace = faceROI;

                // --- Kontrast verstaerken ---
                cv::equalizeHist(grayFace, grayFace);

                cv::resize(grayFace, resizedFace, cv::Size(64, 64));
                cv::Mat blob = cv::dnn::blobFromImage(resizedFace, 1.0F, cv::Size(64, 64), cv::Scalar(), false, false, CV_32F);

                // --- Erkennung ---
                mEmotionNet.setInput(blob);
                cv::Mat prob = mEmotionNet.forward();

                //// --- DEBUG START ---
                //float* rawData = prob.ptr<float>(0);

                //// nur alle 60 Frames ausgeben, damit Konsole nicht explodiert
                //static int debugCounter = 0;
                //if (debugCounter++ % 60 == 0) {
                //    console() << "------------------------------------------------" << std::endl;
                //    console() << "RAW AI OUTPUT (Logits):" << std::endl;
                //    for (int i = 0; i < 8; i++) {
                //        console() << mEmotionLabels[i] << ": " << rawData[i] << std::endl;
                //    }
                //    console() << "------------------------------------------------" << std::endl;
                //}
                //// --- DEBUG ENDE ---



                float* rawScores = prob.ptr<float>(0);
                float maxScore = rawScores[0];
                for (int i = 1; i < 8; i++) if (rawScores[i] > maxScore) maxScore = rawScores[i];

                float sum = 0.0f;
                std::vector<float> expValues(8);
                for (int i = 0; i < 8; i++) {
                    expValues[i] = std::exp(rawScores[i] - maxScore);
                    sum += expValues[i];
                }

                // Werte speichern (0.0 bis 1.0)
                for (int i = 0; i < 8; i++) {
                    mCurrentScores[i] = expValues[i] / sum;
                }

                int winnerIdx = 0; // Default: Neutral

                // --- GRUPPE A: Häufige Emotionen (Happiness, Anger, Surprise) ---

                int activeIdx = 0; ///< neutral
                float activeScore = 0.0f;

                // Happiness (ab 30%)
                if (mCurrentScores[1] > 0.3f && mCurrentScores[1] > activeScore) {
                    activeIdx = 1; activeScore = mCurrentScores[1];
                }
                // Anger (ab 30%)
                if (mCurrentScores[4] > 0.30f && mCurrentScores[4] > activeScore) {
                    activeIdx = 4; activeScore = mCurrentScores[4];
                }
                // Surprise (ab 8%)
                if (mCurrentScores[2] > 0.08f && mCurrentScores[2] > activeScore) {
                    activeIdx = 2; activeScore = mCurrentScores[2];
                }

                // --- GRUPPE B: Seltene Emotionen (Sadness, Fear, Disgust, Contempt) ---
                // "häufiger" -> "seltener"
                // sadness < contempt < fear < disgust

                int trumpIdx = 0; ///< Trumpf = Emotion mit klarer Priorität/ Gewinn lt. Definition
                float trumpScore = 0.0f;

                // Sadness (ab 10%)
                if (mCurrentScores[3] > 0.10f) { trumpIdx = 3; trumpScore = mCurrentScores[3]; }
                
                // Contempt (ab 3%)
                if (mCurrentScores[7] > 0.03f) { trumpIdx = 7; trumpScore = mCurrentScores[7]; }

                // Fear (ab 1% - sehr sensibel, da selten)
                if (mCurrentScores[6] > 0.01f) { trumpIdx = 6; trumpScore = mCurrentScores[6]; }

                // Disgust (ab 2% - sehr sensibel, da selten)
                if (mCurrentScores[5] > 0.02f) { trumpIdx = 5; trumpScore = mCurrentScores[5]; }


                // --- Auswahl der Emotion (Kampf Priorität und Wahrscheinlichkeit) ---

                if (trumpIdx == 0) {
                    // kein Trumpf -> Häufige Emotionen (oder Neutral)
                    winnerIdx = activeIdx;
                }
                else {
                    // Trumpf -> Overrule ist viel schwerer
                    // Der Häufige Emotion gewinnt nur bei > 50% hat UND 20% stärker als Trumpf.
                    if (activeScore > 0.50f && activeScore > (trumpScore + 0.20f)) {
                        winnerIdx = activeIdx; ///< Sieg der Häufigen Emotion
                    }
                    else {
                        winnerIdx = trumpIdx;  ///< Trumpf bleibt Gewinner
                    }
                }

                // --- Ergebnis (Gewinner Emotion) ---
                mCurrentRawIndex = winnerIdx;

                // --- Zeit-Stabilisierung ---
                if (mCurrentRawIndex == mPendingEmotionIndex) {
                    mEmotionTimer++;
                    if (mEmotionTimer > EMOTION_THRESHOLD_FRAMES) {
                        mStableEmotionIndex = mCurrentRawIndex;
                    }
                }
                else {
                    mPendingEmotionIndex = mCurrentRawIndex;
                    mEmotionTimer = 0;
                }
            }

            
        }
        else {
            mHasFace = false;
        }
    }
    catch (const cv::Exception& e) {
        CI_LOG_E("OpenCV Runtime Fehler: " << e.what());
    }
}

void FaceTracker::drawDebug(bool mirrored)
{
    if (mHasFace) {

        // --- Skalierung berechnen ---
        float winW = (float)ci::app::getWindowWidth();
        float winH = (float)ci::app::getWindowHeight();
        
        // Verhältnis berechnen
        float scaleX = winW / mSourceWidth;
        float scaleY = winH / mSourceHeight;

        // --- Koordinaten transformieren ---
        // Skalierung auf Gesicht
        float x = mLastFace.x * scaleX;
        float y = mLastFace.y * scaleY;
        float w = mLastFace.width * scaleX;
        float h = mLastFace.height * scaleY;

        Rectf drawBox = Rectf(x, y, x + w, y + h);

        // --- Spiegeln (falls nötig) ---
        if (mirrored) {
            float x1 = winW - drawBox.x2; ///< Rechts wird Links
            float x2 = winW - drawBox.x1;
            drawBox = Rectf(x1, drawBox.y1, x2, drawBox.y2);
        }

        // --- Rechteck zeichnen ---
        gl::lineWidth(2.0f);
        gl::color(0, 1, 0); ///< Grün, da gut aussieht
        gl::drawStrokedRect(drawBox);

        // --- Infotext & Balken ---
        float barX = drawBox.x2 + 10;
        float barY = drawBox.y1;

        // Rutscht es aus dem Bild?
        if (mirrored || barX > winW - 150) {
            barX = drawBox.x1 - 160; ///< Links daneben zeichnen (bzw rechts wenn gespiegelt)
        }

        for (int i = 0; i < 8; i++) {
            float score = mCurrentScores[i];

            if (i == mStableEmotionIndex) gl::color(1.0f, 0.2f, 0.2f);
            else gl::color(0.8f, 0.8f, 0.8f);

            // --- Text ---
            int percent = (int)(score * 100.0f);

            // --- NaN Schutz (falls 0%) ---
            if (percent < 0) percent = 0;
            if (percent > 100) percent = 100;

            std::string line = mEmotionLabels[i] + ": " + std::to_string(percent) + "%";

            // --- Text zeichnen ---
            gl::drawString(line, vec2(barX, barY + i * 20), ColorA(1, 1, 1, 1), Font("Arial", 14));

            // --- Balken ---
            gl::drawSolidRect(Rectf(barX, barY + i * 20 + 15, barX + score * 80.0f, barY + i * 20 + 18));
        }

        gl::color(1, 1, 1);
        gl::lineWidth(1.0f);
    }
}