#include "DebugView.hpp"
#include "./../blocks/MatToBase64.hpp"

#define _CRTDBG_CHECK_ALWAYS_DF = 1

DebugView::DebugView()
{
	m_camera.setEyePoint(vec3(0.0f, 2.0f, 5.0f));
	m_camera.setPerspective(60, getWindowWidth() / getWindowHeight(), 1, 1000);
	m_camera.lookAt(vec3(0));
	m_cameraUi = CameraUi(&m_camera);
}

DebugView::~DebugView()
{
}

void DebugView::update()
{
}

void DebugView::draw()
{
	gl::clear(Color::black());
	gl::color(Color::white());

	if (m_depth)
		gl::draw(m_depth, Rectf(vec2(0,0), vec2(getWindowCenter().x, getWindowHeight())));
	if(m_bim)
		gl::draw(m_bim, Rectf(vec2(getWindowCenter().x, 0), vec2(getWindowWidth(), getWindowHeight())));
		
	gl::pushMatrices();
	gl::setMatrices(m_camera);

	gl::drawCoordinateFrame();

	gl::color(Color(0.9f, 0.1f, 0.3f));
	for (auto&& body : m_bodies) {
		body->draw();
	}
	gl::popMatrices();
}

void DebugView::mouseDown(MouseEvent event)
{
	m_cameraUi.mouseDown(event);
}

void DebugView::mouseDrag(MouseEvent event)
{
	m_cameraUi.mouseDrag(event);
}

void DebugView::keyUp(KeyEvent event)
{
}

void DebugView::onBodies(act::room::BodyRefList bodies)
{
	m_bodies = bodies;	
}

void DebugView::onIndexMap(cv::UMat map)
{
	m_bim = gl::Texture::create(fromOcv(map));
}

void DebugView::onDepth(cv::UMat depth)
{
	m_depth = gl::Texture::create(fromOcv(depth));
}


