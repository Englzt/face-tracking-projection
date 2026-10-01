/*
	Body Helper - 2024-25

	Do not distribute (including publishing, open-sourcing).
	Usage is only allowed in context of the practical course ("Medieninformatik Projekt", "KP Interaktives Theater") "EarlGrey" (SoSe25) at the Chair for Immersive Media Design (TU Dresden).

	-> ask lars.engeln@tu-dresden.de
*/

#pragma once
#include "KinectReciever.hpp"
#include ".\..\blocks\MatToBase64.hpp"

act::room::KinectReciever::KinectReciever(uint16_t port)
	: m_port(port), m_receiver(port)
{
	m_receiver.setListener("/bodies",
		[&](const osc::Message& msg) {
			std::string data = msg[0].string();
			onBodyMsg(data);
		});
	m_receiver.setListener("/bim",
		[&](const osc::Message& msg) {
			std::string uid		= msg[0].string();
			int totalChunks		= msg[1].int32();
			int chunkIndex		= msg[2].int32();
			std::string data	= msg[3].string();

			if(m_indexMapChunks.find(uid) == m_indexMapChunks.end())
				m_indexMapChunks[uid] = std::vector<ChunkData>();
			m_indexMapChunks[uid].push_back(ChunkData{ uid, totalChunks, chunkIndex, data });

			if(m_indexMapChunks[uid].size() == totalChunks) {
				std::vector<std::string> chunkData(totalChunks);
				for (auto&& chunk : m_indexMapChunks[uid])
					chunkData.push_back(chunk.data);

				std::string mapData = std::accumulate(chunkData.begin(), chunkData.end(), std::string(""));
				onIndexMapMsg(mapData);

				m_indexMapChunks.clear();
			}			
		});
	m_receiver.setListener("/depth",
		[&](const osc::Message& msg) {
			std::string uid		= msg[0].string();
			int totalChunks		= msg[1].int32();
			int chunkIndex		= msg[2].int32();
			std::string data	= msg[3].string();

			if (m_depthMapChunks.find(uid) == m_depthMapChunks.end())
				m_depthMapChunks[uid] = std::vector<ChunkData>();
			m_depthMapChunks[uid].push_back(ChunkData{ uid, totalChunks, chunkIndex, data });

			if (m_depthMapChunks[uid].size() == totalChunks) {
				std::vector<std::string> chunkData(totalChunks);
				for (auto&& chunk : m_depthMapChunks[uid])
					chunkData.push_back(chunk.data);

				std::string mapData = std::accumulate(chunkData.begin(), chunkData.end(), std::string(""));
				onDepthMsg(mapData);

				m_depthMapChunks.clear();
			}
		});
	try {
		// Bind the receiver to the endpoint. This function may throw.
		m_receiver.bind();
	}
	catch (const osc::Exception& ex) {
		CI_LOG_E("Error binding: " << ex.what() << " val: " << ex.value());
		m_isListening = false;
	}

//#if USE_UDP
	// UDP opens the socket and "listens" accepting any message from any endpoint. The listen
	// function takes an error handler for the underlying socket. Any errors that would
	// call this function are because of problems with the socket or with the remote message.
	m_receiver.listen(
		[&](asio::error_code error, protocol::endpoint endpoint) -> bool {
			if (error) {
				CI_LOG_E("Error Listening: " << error.message() << " val: " << error.value() << " endpoint: " << endpoint);
				m_isListening = false;
				return false;
			}
			else {
				m_isListening = true;
				return true;
			}
		});
//#else

}

act::room::KinectReciever::~KinectReciever()
{
}

void act::room::KinectReciever::setOnBodies(std::function<void(BodyRefList)> bodiesCB)
{
	m_bodiesCB = bodiesCB;
	m_hasBodiesCB = true;
}

void act::room::KinectReciever::setOnIndexMap(std::function<void(cv::UMat)> indexMapCB)
{
	m_indexMapCB = indexMapCB;
	m_hasIndexMapCB = true;
}

void act::room::KinectReciever::setOnDepth(std::function<void(cv::UMat)> depthCB)
{
	m_depthCB = depthCB;
	m_hasDepthCB = true;
}

void act::room::KinectReciever::loadTestData()
{
	ci::Json bodies = ci::loadJson(getAssetPath("bodies.json"));
	onBodyMsg(bodies.dump());

	ci::Json depth = ci::loadJson(getAssetPath("depth.json"));
	onDepthMsg(depth["data"]);

	ci::Json bim = ci::loadJson(getAssetPath("bim.json"));
	onIndexMapMsg(bim["data"]);
}

void act::room::KinectReciever::onBodyMsg(std::string data)
{
	if (!m_hasBodiesCB)
		return;

	int count = 0;
	for (char ch : data) {
		if (ch == '{') {
			count++;
		}
		else if (ch == '}') {
			count--;
		}
	}

	if (count != 0)
		return;

	ci::Json json = ci::Json::parse(data);
	if (!json.contains("uid") || !json.contains("index") || !json.contains("amount") || !json.contains("body"))
		return;
	
	std::string uid = json["uid"];
	int totalChunks = json["amount"];
	int chunkIndex = json["index"];
	act::room::BodyRef body = act::room::Body::create();
	body->fromJson(json["body"], true);

	if (m_bodyFrameUID != uid) {
		m_bodyFrameUID = uid;

		if (m_numOfBodies >= totalChunks) {
			m_bodiesCB(m_currentBodies);
		}
		if (totalChunks != m_currentBodies.size())
			m_currentBodies.resize(totalChunks);

		m_numOfBodies = 0;
	}

	m_numOfBodies++;
	m_currentBodies[chunkIndex] = body;
	if(totalChunks == 1)
		m_bodiesCB(m_currentBodies);
}

void act::room::KinectReciever::onIndexMapMsg(std::string data)
{
	if (!m_hasIndexMapCB || data.empty())
		return;

	std::vector<uchar> decoded = base64_decode(data);
	cv::UMat img = cv::imdecode(cv::Mat(decoded), 1).getUMat(cv::ACCESS_FAST);
	m_indexMapCB(img);
}

void act::room::KinectReciever::onDepthMsg(std::string data)
{
	if (!m_hasDepthCB || data.empty())
		return;

	std::vector<uchar> decoded = base64_decode(data);
	cv::UMat img = cv::imdecode(cv::Mat(decoded), 1).getUMat(cv::ACCESS_FAST);
	m_depthCB(img);
}
