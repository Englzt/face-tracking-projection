#pragma once

#include "Body.hpp"
#include "CinderOpenCV.h"

class ViewBase {
	public:
		
		virtual void update() = 0;
		virtual void draw() = 0;

		virtual void onBodies(act::room::BodyRefList bodies) {};
		virtual void onIndexMap(cv::UMat map) {};
		virtual void onDepth(cv::UMat depth) {};

		virtual void mouseDown(MouseEvent event) {};
		virtual void mouseUp(MouseEvent event) {};
		virtual void mouseMove(MouseEvent event) {};
		virtual void mouseDrag(MouseEvent event) {};
		virtual void keyUp(KeyEvent event) {};

	private:
}; using ViewBaseRef = std::shared_ptr<ViewBase>;