/*
	Body Helper - 2024-25

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
#include "cinder/ConcurrentCircularBuffer.h"

#include "CinderOpenCV.h"

namespace act {
	namespace room {

		#define USE_UDP 1

		#if USE_UDP
			using Receiver = osc::ReceiverUdp;
			using protocol = asio::ip::udp;
		#else
			using Receiver = osc::ReceiverTcp;
			using protocol = asio::ip::tcp;
		#endif

		struct ChunkData {
			std::string uid;
			int chunkIndex = 0;
			int totalChunks = 0;
			std::string data;
			ChunkData() {}
			ChunkData(std::string uid, int chunkIndex, int totalChunks, std::string data)
				: uid(uid), chunkIndex(chunkIndex), totalChunks(totalChunks), data(data) {}
		};

		class KinectReciever : public UniqueIDBase {

		public:
			KinectReciever(uint16_t port);
			~KinectReciever();

			static std::shared_ptr<KinectReciever> create(uint16_t port) { return std::make_shared<KinectReciever>(port); };

			bool isListening() { return m_isListening; }
			void setOnBodies(std::function<void(BodyRefList)> bodiesCB);
			void setOnIndexMap(std::function<void(cv::UMat)> indexMapCB);
			void setOnDepth(std::function<void(cv::UMat)> depthCB);

			void loadTestData();

		private:
			uint16_t	m_port;
			Receiver	m_receiver;
			std::map<uint64_t, protocol::endpoint> m_connections;

			bool		m_isListening = false;

			void		onBodyMsg(std::string data);
			void		onIndexMapMsg(std::string data);
			void		onDepthMsg(std::string data);

			bool		m_hasBodiesCB = false;
			bool		m_hasIndexMapCB = false;
			bool		m_hasDepthCB = false;

			std::function<void(BodyRefList)> m_bodiesCB;
			std::function<void(cv::UMat)> m_indexMapCB;
			std::function<void(cv::UMat)> m_depthCB;

			std::string m_bodyFrameUID = "";
			int m_numOfBodies = 0;
			act::room::BodyRefList m_currentBodies;

			std::map<std::string, std::vector<ChunkData>> m_depthMapChunks;
			std::map<std::string, std::vector<ChunkData>> m_indexMapChunks;

		};	using KinectRecieverRef = std::shared_ptr<KinectReciever>;
	}
}