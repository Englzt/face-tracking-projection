/*
	Body Helper - 2025

	Do not distribute (including publishing, open-sourcing).
	Usage is only allowed in context of the practical course ("Medieninformatik Projekt", "KP Interaktives Theater") "EarlGrey" (SoSe25) at the Chair for Immersive Media Design (TU Dresden).

	-> ask lars.engeln@tu-dresden.de
*/

#pragma once
#include "KinectPlayer.hpp"
#include ".\..\blocks\MatToBase64.hpp"
#include <chrono>

act::room::KinectPlayer::KinectPlayer()
{
}

act::room::KinectPlayer::~KinectPlayer()
{
}

void act::room::KinectPlayer::update() {
	if (!m_isPlaying) {
		return;
	}

	auto now = std::chrono::high_resolution_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_playbackStartTime).count();

	if (m_hasBodiesCB) {
		for (m_bodiesPlayPosition; m_bodiesPlayPosition != m_bodiesData.end(); ++m_bodiesPlayPosition) {
			if (m_bodiesPlayPosition->first > elapsed) {
				m_bodiesCB(m_bodiesPlayPosition->second);
				break;
			}
		}
	}
	if (m_hasDepthCB) {
		for (m_depthPlayPosition; m_depthPlayPosition != m_depthData.end(); ++m_depthPlayPosition) {
			if(m_depthPlayPosition->first > elapsed) {
				m_depthCB(m_depthPlayPosition->second);
				break;
			}
		}
	}
	if (m_hasIndexMapCB) {
		for (m_bimPlayPosition; m_bimPlayPosition != m_bimData.end(); ++m_bimPlayPosition) {
			if(m_bimPlayPosition->first > elapsed) {
				m_indexMapCB(m_bimPlayPosition->second);
				break;
			}
		}
	}

	if(m_isLooping && m_bodiesPlayPosition == m_bodiesData.end() && m_depthPlayPosition == m_depthData.end() && m_bimPlayPosition == m_bimData.end()) {
		m_bodiesPlayPosition = m_bodiesData.begin();
		m_depthPlayPosition = m_depthData.begin();
		m_bimPlayPosition = m_bimData.begin();
		m_playbackStartTime = std::chrono::high_resolution_clock::now();
	}

}

void act::room::KinectPlayer::play(fs::path filepath, bool loop)
{
	m_isLooping = loop;
	m_playbackStartTime = std::chrono::high_resolution_clock::now();

	ci::Json json = ci::loadJson(filepath);
	for (auto entry : json) {
		int timestamp = entry["time"];

		if(entry.contains("bodies")) {
			BodyRefList bodies;
			for (auto& bodyJson : entry["bodies"]) {
				act::room::BodyRef body = act::room::Body::create();
				body->fromJson(bodyJson, true);
				bodies.push_back(body);
			}
			m_bodiesData.insert({ timestamp, bodies });
		}
		else if (entry.contains("depth")) {
			cv::UMat indexMap;

			std::vector<uchar> decoded = base64_decode(entry["depth"]);
			cv::UMat img = cv::imdecode(cv::Mat(decoded), 1).getUMat(cv::ACCESS_FAST);

			m_depthData.insert({ timestamp, img });
		}
		else if(entry.contains("bim")) {
			cv::UMat indexMap;

			std::vector<uchar> decoded = base64_decode(entry["bim"]);
			cv::UMat img = cv::imdecode(cv::Mat(decoded), 1).getUMat(cv::ACCESS_FAST);

			m_bimData.insert({ timestamp, img });
		}
	}
	m_bodiesPlayPosition = m_bodiesData.begin();
	m_depthPlayPosition = m_depthData.begin();
	m_bimPlayPosition = m_bimData.begin();
	int i = 0;
	m_isPlaying = true;
}

void act::room::KinectPlayer::setOnBodies(std::function<void(BodyRefList)> bodiesCB)
{
	m_bodiesCB = bodiesCB;
	m_hasBodiesCB = true;
}

void act::room::KinectPlayer::setOnIndexMap(std::function<void(cv::UMat)> indexMapCB)
{
	m_indexMapCB = indexMapCB;
	m_hasIndexMapCB = true;
}

void act::room::KinectPlayer::setOnDepth(std::function<void(cv::UMat)> depthCB)
{
	m_depthCB = depthCB;
	m_hasDepthCB = true;
}