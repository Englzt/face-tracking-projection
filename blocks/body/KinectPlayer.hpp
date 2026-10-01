/*
	Body Helper - 2025

	Do not distribute (including publishing, open-sourcing).
	Usage is only allowed in context of the practical course ("Medieninformatik Projekt", "KP Interaktives Theater") "EarlGrey" (SoSe25) at the Chair for Immersive Media Design (TU Dresden).

	-> ask lars.engeln@tu-dresden.de
*/

#pragma once
#include "cinder/app/App.h"
#include "Body.hpp"
#include "UniqueIDBase.hpp"

#include "cinder/osc/Osc.h"
#include "cinder/Log.h"

#include "CinderOpenCV.h"

namespace act {
	namespace room {

		class KinectPlayer : public UniqueIDBase {

		public:
			KinectPlayer();
			~KinectPlayer();

			static std::shared_ptr<KinectPlayer> create() { return std::make_shared<KinectPlayer>(); };

			void update();

			void play(fs::path filepath, bool loop = true);
			void setOnBodies(std::function<void(BodyRefList)> bodiesCB);
			void setOnIndexMap(std::function<void(cv::UMat)> indexMapCB);
			void setOnDepth(std::function<void(cv::UMat)> depthCB);

		private:
			bool m_isPlaying = false;
			bool m_isLooping = false;
			std::chrono::high_resolution_clock::time_point m_playbackStartTime;
			std::map<int, BodyRefList>::iterator m_bodiesPlayPosition;
			std::map<int, cv::UMat>::iterator m_depthPlayPosition;
			std::map<int, cv::UMat>::iterator m_bimPlayPosition;

			std::map<int, BodyRefList> m_bodiesData;
			std::map<int, cv::UMat> m_depthData;
			std::map<int, cv::UMat> m_bimData;

			bool		m_hasBodiesCB = false;
			bool		m_hasIndexMapCB = false;
			bool		m_hasDepthCB = false;

			std::function<void(BodyRefList)>	m_bodiesCB;
			std::function<void(cv::UMat)>		m_indexMapCB;
			std::function<void(cv::UMat)>		m_depthCB;

		};	using KinectPlayerRef = std::shared_ptr<KinectPlayer>;
	}
}