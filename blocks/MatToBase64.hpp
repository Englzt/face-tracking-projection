#include "cinder/app/App.h"
#include "CinderOpenCV.h" 

#ifndef _BASE64_H_
#define _BASE64_H_

#include <vector>
#include <string>
typedef unsigned char BYTE;

std::string base64_encode(std::vector<BYTE> buf);
std::vector<BYTE> base64_decode(std::string const&);

std::string  surface8uToBase64(ci::Surface8u imgSurface8u, cv::String ext);
std::string  matToBase64(cv::Mat imgMat, cv::String ext, int quality = 70, bool scale = false, int newWidth = 1280);
#endif