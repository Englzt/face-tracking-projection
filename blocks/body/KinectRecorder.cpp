/*
	Body Helper - 2025

	Do not distribute (including publishing, open-sourcing).
	Usage is only allowed in context of the practical course ("Medieninformatik Projekt", "KP Interaktives Theater") "EarlGrey" (SoSe25) at the Chair for Immersive Media Design (TU Dresden).

	-> ask lars.engeln@tu-dresden.de
*/

#pragma once
#include "KinectRecorder.hpp"
#include ".\..\blocks\MatToBase64.hpp"

act::room::KinectRecorder::KinectRecorder(int quality)
{
	std::clamp(quality, 0, 100);
	m_recordingQuality = quality;
}

act::room::KinectRecorder::~KinectRecorder()
{
}


void act::room::KinectRecorder::startRecording()
{
	m_isRecording = true;
	m_recordingJson = ci::Json::array();
	m_recordingPath = app::getAssetPath("testdata.json");
	// m_recordingPath = app::getSaveFilePath(app::getAssetPath("testdata.json"), { "*.json" });
	m_recordingStartTime = std::chrono::high_resolution_clock::now();
}

void act::room::KinectRecorder::stopRecording()
{
	m_isRecording = false;
	ci::writeJson(m_recordingPath, m_recordingJson);
}

void act::room::KinectRecorder::record(std::string key, cv::UMat value)
{
	if (m_isRecording) {
		auto now = std::chrono::high_resolution_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_recordingStartTime).count();
		cv::Mat mat = value.getMat(cv::ACCESS_FAST).clone();
		std::string data = matToBase64(mat, ".jpg", m_recordingQuality, false);
		ci::Json mapJson = ci::Json::object();
		mapJson["time"] = elapsed;
		mapJson[key] = data;
		m_recordingJson.push_back(mapJson);
	}
}

void act::room::KinectRecorder::recordBodies(act::room::BodyRefList bodies)
{
	if (m_isRecording) {
		auto now = std::chrono::steady_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_recordingStartTime).count();
		m_recordingJson.push_back(ci::Json::object({ {"time", elapsed}, {"bodies", ci::Json::array()} }));
		for (const auto& body : bodies) {
			m_recordingJson.back()["bodies"].push_back(body->toJson());
		}
	}
}
