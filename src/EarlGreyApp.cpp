#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/CinderImGui.h"
#include "cinder/Capture.h"

#include "KinectReciever.hpp"
#include "KinectRecorder.hpp"
#include "KinectPlayer.hpp"
#include "DebugView.hpp"
#include "ProjectorView.hpp"
#include "FacePreview.hpp"

using namespace ci;
using namespace ci::app;
using namespace std;

#define MULTIWINDOW = 0

class EarlGreyApp : public App {
	public:
		std::shared_ptr<FacePreview> getFacePreview() const { return mFacePreview; }
		void setup() override;
		void mouseDown(MouseEvent event) override;
		void mouseUp(MouseEvent event) override;
		void mouseMove(MouseEvent event) override;
		void mouseDrag(MouseEvent event) override;
		void keyUp( KeyEvent event ) override;
		void update() override;
		void draw() override;

	private:
	  std::shared_ptr<FacePreview> mFacePreview;
	  act::room::KinectRecieverRef m_reciever;
	  act::room::KinectRecorderRef m_recorder;
	  act::room::KinectPlayerRef m_player;

	  // --- Member ---
	  ViewBaseRef m_debugView;
	  ViewBaseRef m_projectorView;
	  ViewBaseRef m_view;
	  bool m_showDebug = true;


	  // --- Fenster-Referenzen & Umschalter ---
	  ci::app::WindowRef m_previewWin; ///< links – FacePreview
	  ci::app::WindowRef m_beamerWin;  ///< rechts – Projector/Preview
	  bool m_beamerShowsMask = true;   ///< toggelt mit Taste D

	  void setView();

	  void onBodies(act::room::BodyRefList bodies);
	  void onIndexMap(cv::UMat map);
	  void onDepth(cv::UMat depth);
};

void EarlGreyApp::setup()
{
	ImGui::Initialize();

	// --- Preview-Fenster (links) ---
	m_previewWin = getWindow(); ///< Referenz merken
	m_previewWin->setTitle("Preview");
	m_previewWin->setSize(1920, 1080);
	m_previewWin->setPos(10, 100); ///< etwas nach unten und nach rechts verschieben

	// --- eigenes Draw-Signal: immer FacePreview ---
	m_previewWin->getSignalDraw().connect([this] {gl::clear(ci::Color::black()); if (mFacePreview) mFacePreview->draw(); });

	// --- Beamer-Fenster (rechts) ---
	m_beamerWin = createWindow(ci::app::Window::Format().size(1920, 1080).title("Projector"));
	m_beamerWin->setPos(ci::vec2(660, 100)); ///< rechts neben Preview fenster & etwas nach unten verschieben

	// --- Draw-Signal: startend mit Maske ---
	m_beamerWin->getSignalDraw().connect([this] {
		gl::clear(ci::Color::black());
		if (m_beamerShowsMask && m_projectorView)
			m_projectorView->draw();
		else if (mFacePreview)
			mFacePreview->draw();
		});


	m_debugView = DebugView::create();
	m_projectorView = ProjectorView::create();
	auto projView = std::static_pointer_cast<ProjectorView>(m_projectorView);
	projView->setup();


	setView();

	m_reciever = act::room::KinectReciever::create(9999);
	m_reciever->setOnBodies([&](act::room::BodyRefList bodies) { onBodies(bodies); });
	m_reciever->setOnIndexMap([&](cv::UMat map) { onIndexMap(map); });
	m_reciever->setOnDepth([&](cv::UMat depth) { onDepth(depth); });

	m_recorder = act::room::KinectRecorder::create(90);
	m_player = act::room::KinectPlayer::create();
	m_player->setOnBodies([&](act::room::BodyRefList bodies) { onBodies(bodies); });
	m_player->setOnIndexMap([&](cv::UMat map) { onIndexMap(map); });
	m_player->setOnDepth([&](cv::UMat depth) { onDepth(depth); });

	//ci::Json bodyJson = ci::loadJson(loadFile("sample_body-data.json"));

	//onBodies();

	mFacePreview = FacePreview::create();
	mFacePreview->setup();

	projView->setFacePreview(mFacePreview);
	mFacePreview->setProjectorView(projView.get());
}


void EarlGreyApp::mouseDown(MouseEvent event)
{
	m_view->mouseDown(event);
}

void EarlGreyApp::mouseUp( MouseEvent event )
{
	m_view->mouseUp(event);
}

void EarlGreyApp::mouseMove(MouseEvent event)
{
	m_view->mouseMove(event);
}

void EarlGreyApp::mouseDrag(MouseEvent event)
{
	m_view->mouseDrag(event);
}

void EarlGreyApp::keyUp(KeyEvent event)
{
	switch (event.getCode()) {
		case KeyEvent::KEY_ESCAPE:
			app::getWindow()->close();
			break;
		case KeyEvent::KEY_f:
			app::getWindow()->setFullScreen();
			break;
		case KeyEvent::KEY_d: ///< nur Beamer Fenster zwischen Maske und Preview wechseln
			m_beamerShowsMask = !m_beamerShowsMask;
			m_beamerWin->setTitle(m_beamerShowsMask ? "Projector (Mask)"
				: "Projector (Preview)");
			break;
		case KeyEvent::KEY_t:
			m_reciever->loadTestData();
			m_player->play(app::getAssetPath("testdata.json"));
			break;
		case KeyEvent::KEY_r:
			if (!m_recorder->isRecording()) {
				m_recorder->startRecording();
			}
			else {
				m_recorder->stopRecording();
			}
			break;

			// --- Maske wechseln k & l ---
		case KeyEvent::KEY_l:
			std::static_pointer_cast<ProjectorView>(m_projectorView)->nextMask();
			break;
		case KeyEvent::KEY_k:
			std::static_pointer_cast<ProjectorView>(m_projectorView)->prevMask();
			break;

			// --- Verschieben mit Pfeiltasten ---
		case KeyEvent::KEY_UP:
			std::static_pointer_cast<ProjectorView>(m_projectorView)->adjustOffset(vec2(0, -20));
			break;
		case KeyEvent::KEY_DOWN:
			std::static_pointer_cast<ProjectorView>(m_projectorView)->adjustOffset(vec2(0, 20));
			break;
		case KeyEvent::KEY_LEFT:
			std::static_pointer_cast<ProjectorView>(m_projectorView)->adjustOffset(vec2(-20, 0));
			break;
		case KeyEvent::KEY_RIGHT:
			std::static_pointer_cast<ProjectorView>(m_projectorView)->adjustOffset(vec2(20, 0));
			break;

			// --- Zoom mit Komma & Punkt ---
		case KeyEvent::KEY_COMMA: ///< Kleiner
			std::static_pointer_cast<ProjectorView>(m_projectorView)->adjustScale(-0.1f);
			break;
		case KeyEvent::KEY_PERIOD: ///< Größer
			std::static_pointer_cast<ProjectorView>(m_projectorView)->adjustScale(0.1f);
			break;

			// --- Auflösung beider Fentster umschalten --- 
		case KeyEvent::KEY_1:
			if (m_previewWin && m_beamerWin) {
				m_previewWin->setSize(1920, 1080);
				m_beamerWin->setSize(1920, 1080);
			}
			break;
		case KeyEvent::KEY_2:
			if (m_previewWin && m_beamerWin) {
				m_previewWin->setSize(1280, 720);
				m_beamerWin->setSize(1280, 720);
			}
			break;
		case KeyEvent::KEY_3:
			if (m_previewWin && m_beamerWin) {
				m_previewWin->setSize(640, 480);
				m_beamerWin->setSize(640, 480);
			}
			break;

			// --- erzwingt sofortigen maskenwechsel ---
		case KeyEvent::KEY_s:
			if (mFacePreview) {
				mFacePreview->getTracker().forceEmotionNow();
				console() << "Emotion forced!" << std::endl;
			}
			break;

		default:
			m_view->keyUp(event);
			break;
	}
}

void EarlGreyApp::update()
{
	m_player->update();
	m_view->update();
	if (m_projectorView) m_projectorView->update();
	if (mFacePreview) mFacePreview->update();
}

void EarlGreyApp::draw() ///< ersetzt um nicht doppelt 
{ 
	/*gl::pushMatrices();
	m_view->draw();
	gl::popMatrices();

	gl::color(Color::white());

	if (m_recorder->isRecording()) {
		gl::drawString("Recording...", app::getWindowCenter(), Color(1.0f, 0.1f, 0.2f), Font("Arial", 32));
	}
	else {
		gl::drawString("Press 'r' to start recording", vec2(10, 10), Color::white(), Font("Arial", 20));
	}
	if (m_showDebug && mFacePreview) mFacePreview->draw();*/
}

void EarlGreyApp::setView()
{
	if (m_showDebug) {
		m_view = m_debugView;
		app::getWindow()->setTitle("EarlGrey - debugView");
	}
	else {
		m_view = m_projectorView;
		app::getWindow()->setTitle("EarlGrey - projectorView");
	}
}

void EarlGreyApp::onBodies(act::room::BodyRefList bodies)
{
	m_recorder->recordBodies(bodies);
	m_view->onBodies(bodies);
}

void EarlGreyApp::onIndexMap(cv::UMat map)
{
	m_recorder->recordIndexMap(map);
	m_view->onIndexMap(map);
}

void EarlGreyApp::onDepth(cv::UMat depth)
{
	m_recorder->recordDepth(depth);
	m_view->onDepth(depth);
}
// --- Fuer Debug Window (in Tracker) ---
//void prepareSettings(EarlGreyApp::Settings* settings)
//{
//	settings->setConsoleWindowEnabled(true);
//}
CINDER_APP( EarlGreyApp, RendererGl ) ///< prepareSettings rein fuer debug Konsolen Fenster