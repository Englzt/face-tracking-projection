#pragma once

#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Rand.h"
#include "cinder/ImageIO.h"
#include "cinder/Log.h"

#include "ViewBase.hpp"
#include "FacePreview.hpp"

using namespace ci;
using namespace ci::app;
using namespace std;

/**
 * Zeigt die Maske auf dem Projektor‑Fenster.
 * – `setup()` lädt PNG‑Maske.
 * – `draw()` positioniert & skaliert Maske über dem erkannten Gesicht.
 */
class ProjectorView : public ViewBase {
public:
	ProjectorView();
	~ProjectorView();

	static std::shared_ptr<ProjectorView> create() { return std::make_shared<ProjectorView>(); };

	void setup();
	void update() override; ///< reserviert
	void draw() override;

	// --- Tracker Quelle ---
	void setFacePreview(std::shared_ptr<FacePreview> fp) { mFacePreview = fp; }

	void nextMask(); ///< nächste Maske n
	void prevMask(); ///< vorige Maske n

	ci::gl::TextureRef getCurrentMask() const; ///< aktuell aktive Maske

	// --- Funktionen zum Steuern ---
	void adjustOffset(ci::vec2 delta) { mOffset += delta; }
	void adjustScale(float delta) { mScaleMult += delta; }

	// --- Getter für FacePreview, damit die Maske dort synchron ist ---
	ci::vec2 getOffset() const { return mOffset; }
	float    getScaleMult() const { return mScaleMult; }

	// --- Zeug für den Shader ---
	float				mCurrentRun;
	int					mElapsedTime;
	gl::GlslProgRef		mGlsl;
	gl::TextureRef		mTexture;
	gl::BatchRef		bTex;

	void updateShader();
	gl::GlslProgRef getShader();
	int ProjectorView::getMaskIdx();

private: 
	std::vector<ci::gl::TextureRef> m_masks;   ///< alle geladenen Masken 
	size_t                          m_maskIdx = 0; ///< aktuell aktive Maske
	std::shared_ptr<FacePreview>    mFacePreview;

	ci::vec2  mOffset = { 0.0f, 0.0f };
	float     mScaleMult = 2.0f; ///< scaling startwert
	
};
