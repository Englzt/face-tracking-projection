#pragma once

#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/CameraUi.h"

#include "ViewBase.hpp"

using namespace ci;
using namespace ci::app;
using namespace std;

class DebugView : public ViewBase {
public:
	DebugView();
	~DebugView();

	static std::shared_ptr<DebugView> create() { return std::make_shared<DebugView>(); };

	void update() override;
	void draw() override;

	void mouseDown(MouseEvent event) override;
	void mouseDrag(MouseEvent event) override;

	void keyUp(KeyEvent event) override;
    
	void onBodies(act::room::BodyRefList bodies) override;
	void onIndexMap(cv::UMat map) override;
	void onDepth(cv::UMat depth) override;

private:
	act::room::BodyRefList m_bodies;

	ci::gl::TextureRef	m_depth;
	ci::gl::TextureRef	m_bim;

	ci::CameraPersp		m_camera;
	ci::CameraUi		m_cameraUi;
};
