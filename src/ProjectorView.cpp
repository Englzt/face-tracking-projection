#include "ProjectorView.hpp"

#include "cinder/ImageIo.h"
#include "cinder/Filesystem.h"

#include "cinder/Rand.h"
#include "cinder/Timeline.h"

#include <algorithm> 

using namespace ci;
using namespace ci::app;

ProjectorView::ProjectorView(){}

ProjectorView::~ProjectorView(){}

// ---------------------------------------------------------------------------
//  Maske(n) laden
// ---------------------------------------------------------------------------
void ProjectorView::setup()
{
    try {
        // --- alle PNG-Dateien im Ordner masks/ einlesen ---
        fs::path dir = getAssetPath("") / "masks";
        int num = 0;
        for (auto& p : fs::directory_iterator(dir))
            if (p.path().extension() == ".png") {
                auto img = loadImage(p.path());
                auto mTex = gl::Texture2d::create(img);
                m_masks.push_back(mTex);
                //mTex->bind(num);
                num++;
            }
                

        if (m_masks.empty())
            CI_LOG_E("Keine Masken gefunden in assets/masks/*.png");
    }
    catch (const std::exception& e) {
        CI_LOG_E("Masken konnten nicht geladen werden: " << e.what());
    }

    // ---Shader Set-Up ---
    mCurrentRun = 0;
    mElapsedTime = 1;

    mGlsl = gl::GlslProg::create(gl::GlslProg::Format()
        .vertex(CI_GLSL(150,
            uniform mat4	ciModelViewProjection;
            in vec4			ciPosition;

            in vec2			ciTexCoord0;
            out vec2		TexCoord0;

            uniform float  elapsed;

            float offsetY(vec2 uv)
            {
                return (sin(uv.x *  elapsed) +
                    cos(uv.y * 7.0f + uv.x * 13.0f)) * 0.9f;
            }

            float offsetX(vec2 uv)
            {
                return (sin(uv.y * 7.0f + uv.x * 13.0f) +
                    cos(uv.x * elapsed)) * 0.9f;
            }

            void main(void) {
                vec4 pos = ciPosition;
                pos.y = offsetY(ciTexCoord0);
                pos.x = offsetX(ciTexCoord0);
                vec4 positionVec4 = ciModelViewProjection * ciPosition;
                positionVec4.y = positionVec4.y + pos.y * 0.05;
                positionVec4.x = positionVec4.x + pos.x * 0.01;
                
                gl_Position = positionVec4;
                TexCoord0 = ciTexCoord0;
            }
        ))
        .fragment(CI_GLSL(150,
            uniform sampler2D	uTex0;
            uniform float		uCheckSize;

            in vec2				TexCoord0;
            out vec4			oColor;

           

            void main(void) {
                oColor = texture(uTex0, TexCoord0);
            }
        )));

}


void ProjectorView::update() {
    if (!mFacePreview) return;
    auto& tracker = mFacePreview->getTracker();

    // Maskenwahl via KI (hier damit es immer läuft)
    if (tracker.hasFace()) {
        int emoIdx = tracker.getCurrentEmotionIndex();
        if (emoIdx >= 0 && emoIdx < m_masks.size()) {
            m_maskIdx = emoIdx;
        }
    }
}

// ---------------------------------------------------------------------------
//  Maske auf Gesicht anheften & zeichnen
// ---------------------------------------------------------------------------
void ProjectorView::draw()
{
    gl::clear(ci::Color::black());

    if (!mFacePreview) return;
    auto& tracker = mFacePreview->getTracker();
    const cv::Rect2f& face = tracker.getSmoothFace();

    // --- Wenn kein Gesicht da ist -> Abbruch (Schwarzer Bildschirm) ---
    if (face.width <= 0) return;

    ci::vec2 winSize = getWindowSize();

    // --- Koordinaten Berechnung ---
    float camW = 1920.0f;
    float baseScale = winSize.x / camW;
    float faceCenterX = face.x + face.width * 0.5f;
    float faceCenterY = face.y + face.height * 0.5f;

    ci::vec2 center = {
        faceCenterX * baseScale + mOffset.x,
        faceCenterY * baseScale + mOffset.y
    };

    ci::gl::TextureRef currentMask = getCurrentMask();

    // --- Nur Maske zeichnen ---
    if (currentMask) {
        float faceWidthOnScreen = face.width * baseScale;
        float scale = (faceWidthOnScreen / (float)currentMask->getWidth()) * mScaleMult;

        ci::vec2 drawSize = vec2(currentMask->getSize()) * scale;
        ci::vec2 pos = center - drawSize * 0.5f;
        ci::Rectf drawRect(pos, pos + drawSize);

        updateShader();

        // --- Shader anwenden (ohne Batch) ---
        if (mGlsl) {
            gl::ScopedGlslProg scpGlsl(mGlsl);
            mGlsl->uniform("elapsed", mCurrentRun);
            mGlsl->uniform("uTex0", 0);

            gl::ScopedTextureBind scpTex(currentMask, 0);

            gl::enableAlphaBlending();
            gl::color(ci::Color::white());
            gl::drawSolidRect(drawRect); ///< Kein Batch = Kein Freeze
            gl::disableAlphaBlending();
        }
        else {
            // --- Fallback falls Shader kaputt (für Kalibrierung eigentlich) ---
            gl::ScopedTextureBind scpTex(currentMask);
            gl::draw(currentMask, drawRect);
        }
    }
}

// ---------------------------------------------------------------------------
//  Shader Getter + Update
// ---------------------------------------------------------------------------

void ProjectorView::updateShader() {
    if (mElapsedTime == 2) {
        mCurrentRun = mCurrentRun + 0.5;
        if (mCurrentRun > acos(-1.0)) {
            mCurrentRun = 0;
        }
        mElapsedTime = 1;
    }
    else
    {
        mElapsedTime++;
    }
    int idx = m_maskIdx;

    mGlsl->uniform("uTex0", idx);
    mGlsl->uniform("uCheckSize", 30.0f);
    mGlsl->uniform("elapsed", mCurrentRun);
}

gl::GlslProgRef ProjectorView::getShader() {

    return mGlsl;
}

int ProjectorView::getMaskIdx() {
    int idx = m_maskIdx;

    return idx;
}

// ---------------------------------------------------------------------------
//  Getter + Umschalten (Maskenumschalten momentan nicht da)
// ---------------------------------------------------------------------------
ci::gl::TextureRef ProjectorView::getCurrentMask() const
{
    if (m_masks.empty()) return nullptr;
    int idx = (m_maskIdx % (int)m_masks.size() + (int)m_masks.size()) % (int)m_masks.size();
    return m_masks[idx];
}

void ProjectorView::nextMask()
{
    if (!m_masks.empty()) m_maskIdx = (m_maskIdx + 1) % (int)m_masks.size();
}
void ProjectorView::prevMask()
{
    if (!m_masks.empty()) m_maskIdx = (m_maskIdx - 1 + (int)m_masks.size()) % (int)m_masks.size();
}

// alter Debug Stuff
	/*gl::color(ci::Color(ci::randVec3()));
	for(int i = 0; i < 5; i++)
		gl::drawSolidCircle(ci::randVec2() * (getWindowHeight() * 0.3f) + getWindowCenter(), ci::randFloat() * 42);*/

