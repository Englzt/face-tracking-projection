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
#include <chrono>

namespace act {
	namespace room {

		class KinectRecorder : public UniqueIDBase {

		public:
			KinectRecorder(int quality);
			~KinectRecorder();

			static std::shared_ptr<KinectRecorder> create(int quality = 89) { return std::make_shared<KinectRecorder>(quality); };

			bool isRecording() const { return m_isRecording; }

			void startRecording();
			void stopRecording();

			void recordDepth(cv::UMat depth) { record("depth", depth); };
			void recordIndexMap(cv::UMat indexMap) { record("bim", indexMap); };
			void recordBodies(act::room::BodyRefList bodies);

		private:
			bool			m_isRecording = false;
			fs::path		m_recordingPath;
			ci::Json 		m_recordingJson;
			std::chrono::high_resolution_clock::time_point m_recordingStartTime;

			int m_recordingQuality = 89; // 0-100, 100 is lossless

			void record(std::string key, cv::UMat value);

		};	using KinectRecorderRef = std::shared_ptr<KinectRecorder>;
	}
}