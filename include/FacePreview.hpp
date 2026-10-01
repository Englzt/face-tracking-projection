#pragma once
class ProjectorView;
class masks;

#include "cinder/app/App.h"
#include "cinder/gl/gl.h"
#include "cinder/Capture.h"
#include "cinder/Log.h"

#include "FaceTracker.hpp"

/**
 * zeigt Kamerabild & dient als Datenquelle für Tracker
 */
class FacePreview {
public:
    static std::shared_ptr<FacePreview> create();

    void setup(); ///< Kamera öffnen & Tracker initialisieren
    void update(); ///< neues Frame holen + Tracker updaten
    void draw(); ///< Bild + Debug zeichnen

    void setProjectorView(ProjectorView* pv) { m_projView = pv; } ///< Setter

    FaceTracker& getTracker() { return mTracker; } ///< Zugriff für ProjectorView

private:
    ci::CaptureRef mCapture; ///< Kamera
    ci::gl::TextureRef mTex; ///< aktuelles Kamera‑Frame als GL‑Textur
    FaceTracker mTracker;
    ProjectorView* m_projView = nullptr; ///< Referenz auf Projektor n
};

