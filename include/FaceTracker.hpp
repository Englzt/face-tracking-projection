#pragma once
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp> // DNN Modul für die KI
#include "cinder/Cinder.h"
#include "cinder/gl/gl.h"
#include "cinder/Surface.h"          
#include "CinderOpenCV.h"

/**
 * NEU: AI-Powered Face Tracker
 * - Nutzt YuNet für das Tracking
 * - Nutzt FER+ für die Emotionserkennung
 */
class FaceTracker {
public:
    void setup(); ///< Modelle laden (ONNX)
    void update(const ci::Surface8u& src); ///< Frame rein, Daten raus

    bool hasFace() const { return mHasFace; }
    cv::Rect2f getSmoothFace() const { return mLastFace; }

    // --- Getter für die Emotionen ---
    int getCurrentEmotionIndex() const { return mStableEmotionIndex; }
    std::string getCurrentEmotionLabel() const { return mEmotionLabels[mStableEmotionIndex]; }

    // ---  gibt werte der Emotionen zurueck ---
    float getCurrentConfidence() const { return mCurrentScores[mStableEmotionIndex]; }

    void drawDebug(bool mirrored = false);

    // --- erzwingt Maskenwechsel
    void forceEmotionNow() {
        mStableEmotionIndex = mCurrentRawIndex;
        mPendingEmotionIndex = mCurrentRawIndex;
        mEmotionTimer = 0;
    }

private:
    // --- KI Modelle (OpenCV DNN) ---
    cv::Ptr<cv::FaceDetectorYN> mFaceDetector; ///< YuNet (statt Cascade)
    cv::dnn::Net                mEmotionNet;   ///< FER+ fuer Emotionserkennung

    // --- Status ---
    bool        mHasFace = false;
    cv::Rect2f  mLastFace;

    // --- Kameraausfloesung fallback ---
    float       mSourceWidth = 1920.0f;
    float       mSourceHeight = 1080.0f;

    // --- Emotion Data ---
    std::string mCurrentEmotionLabel = "neutral";
    int         mCurrentRawIndex = 0;         ///< Emotion jetzt
    int         mStableEmotionIndex = 0;      ///< Was anzeigen
    int         mPendingEmotionIndex = 0;     ///< Anwärter Emotion
    int         mEmotionTimer = 0;            ///< Wie lange der Anwärter wartet
    const int   EMOTION_THRESHOLD_FRAMES = 8; ///< Nach 8 Frames umschalten

    // --- Emotionswerte ---
    float mCurrentScores[8] = { 0.0f };

    // --- Reihenfolge im FER+ Modell ---
    const std::vector<std::string> mEmotionLabels = {
        "neutral", "happiness", "surprise", "sadness",
        "anger", "disgust", "fear", "contempt"
    };

    // --- Bewegungsglättung (0.1 = weich/ langsam, 0.5 = schnell) ---
    const float SMOOTH_FACTOR = 0.15f;
};