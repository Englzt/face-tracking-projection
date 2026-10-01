#include "FacePreview.hpp"
#include "ProjectorView.hpp"

using namespace ci; using namespace ci::app;

std::shared_ptr<FacePreview> FacePreview::create() { return std::make_shared<FacePreview>(); }

// ---------------------------------------------------------------------------
//  Kamera initialisieren & FaceTracker vorbereiten
// ---------------------------------------------------------------------------
void FacePreview::setup()
{
    // --- FaceTracker initialisieren ---
    mTracker.setup();
    
    // --- Kamera-Gerät suchen ---
    auto devices = Capture::getDevices();
    if (devices.empty()) {
        CI_LOG_E("Keine Kamera gefunden!");
        return;
    }

    try { 
        mCapture = ci::Capture::create(1920, 1080, devices[0]); mCapture->start(); 
    }
    catch (...) { 
        CI_LOG_E("Kamera Fehler!"); 
    }
    
}

// -----------------------------------------------------------------------------
//  neues Frame holen, Texture & Tracking aktualisieren
// -----------------------------------------------------------------------------
void FacePreview::update()
{
    if (!mCapture) return;

    if (mCapture->checkNewFrame()) {
        auto surf = mCapture->getSurface();
        mTex = gl::Texture2d::create(*surf);

        mTracker.update(*surf);
    }
}



void FacePreview::draw()
{
    if (!mTex) return;

    // --- Kamerabild zeichnen (GESPIEGELT) --
    gl::pushModelMatrix();
    gl::translate(getWindowWidth(), 0);
    gl::scale(-1.0f, 1.0f);
    gl::color(1, 1, 1);
    gl::draw(mTex, getWindowBounds());
    gl::popModelMatrix();

    // --- Maske zeichnen ---
    if (m_projView && mTracker.hasFace())
    {
        const cv::Rect2f& face = mTracker.getSmoothFace();
        auto maskTex = m_projView->getCurrentMask();

        if (maskTex && face.width > 0)
        {
            float scaleX = getWindowWidth() / 1920.0f;
            float scaleY = getWindowHeight() / 1080.0f;

            float scaledFaceX = face.x * scaleX;
            float scaledFaceY = face.y * scaleY;
            float scaledFaceW = face.width * scaleX;
            float scaledFaceH = face.height * scaleY;
            float mirroredX = getWindowWidth() - (scaledFaceX + scaledFaceW);

            float zoomFactor = m_projView->getScaleMult();
            ci::vec2 offset = m_projView->getOffset();
            float scaledOffsetX = (offset.x * scaleX) * -1.0f;
            float scaledOffsetY = (offset.y * scaleY);

            ci::vec2 localCenter = {
                mirroredX + scaledFaceW * 0.5f + scaledOffsetX,
                scaledFaceY + scaledFaceH * 0.5f + scaledOffsetY
            };

            float scale = (scaledFaceW / (float)maskTex->getWidth()) * zoomFactor;
            ci::vec2 drawSize = vec2(maskTex->getSize()) * scale;
            ci::vec2 pos = localCenter - drawSize * 0.5f;
            ci::Rectf drawRect(pos, pos + drawSize);

            // --- Shader holen ---
            gl::GlslProgRef shdr = m_projView->getShader();

            if (shdr) {
                {
                    gl::ScopedGlslProg scpGlsl(shdr);
                    // --- Zeit synchronisieren ---
                    shdr->uniform("elapsed", m_projView->mCurrentRun);
                    shdr->uniform("uTex0", 0);

                    // --- Textur binden ---
                    gl::ScopedTextureBind scpTex(maskTex, 0);

                    gl::enableAlphaBlending();
                    gl::color(1, 1, 1, 0.5f);
                    gl::drawSolidRect(drawRect);
                    gl::disableAlphaBlending();
                }
            }
        }
    }
    mTracker.drawDebug(true);
}