/*
    Body Helper - 2024-25

    Do not distribute (including publishing, open-sourcing).
    Usage is only allowed in context of the practical course ("Medieninformatik Projekt", "KP Interaktives Theater") "EarlGrey" (SoSe25) at the Chair for Immersive Media Design (TU Dresden).

    -> ask lars.engeln@tu-dresden.de
*/

#pragma once

#include <string>
#include <sstream>
#include <memory>
#include <vector>

#include "cinder/app/App.h"
#include "cinder/Vector.h"

#include "cinder/Json.h"

using namespace ci;

namespace act {
	namespace util {


        static bool setValueFromJson(ci::Json json, std::string key, int& value) {
            if (json.contains(key)) {
                try {
                    value = json[key];
                }
                catch (...) {
                    return false;
                }
                return true;
            }
            return false;
        }

        static bool setValueFromJson(ci::Json json, std::string key, float& value) {
            if (json.contains(key)) {
                try {
                    if (json[key].type() == ci::Json::value_t::string) {
                        value = std::stof((std::string)json[key]);
                    }
                    else
                        value = json[key];
                }
                catch (...) {
                    return false;
                }
                return true;
            }
            return false;
        }

        static bool setValueFromJson(ci::Json json, std::string key, double& value) {
            if (json.contains(key)) {
                try {
                    value = json[key];
                }
                catch (...) {
                    return false;
                }
                return true;
            }
            return false;
        }

        static bool setValueFromJson(ci::Json json, std::string key, std::string& value) {
            if (json.contains(key)) {
                try {
                    value = json[key];
                }
                catch (...) {
                    return false;
                }
                return true;
            }
            return false;
        }

        static bool setValueFromJson(ci::Json json, std::string key, bool& value) {
            if (json.contains(key)) {
                try {
                    value = (bool)(json[key]);
                }
                catch (...) {
                    return false;
                }
                return true;
            }
            return false;
        }

        static bool setValueFromJson(ci::Json json, std::string key, vec2& value) {
            if (json.contains(key)) {
                try {
                    auto vec = json[key];
                    return setValueFromJson(vec, "x", value.x) && setValueFromJson(vec, "y", value.y);
                }
                catch (...) {
                    return false;
                }
                return true;
            }
            return false;
        }

        static bool setValueFromJson(ci::Json json, std::string key, ivec2& value) {
            if (json.contains(key)) {
                try {
                    auto vec = json[key];
                    return setValueFromJson(vec, "x", value.x) && setValueFromJson(vec, "y", value.y);
                }
                catch (...) {
                    return false;
                }
                return true;
            }
            return false;
        }

        static bool setValueFromJson(ci::Json json, std::string key, vec3& value) {
            if (json.contains(key)) {
                try {
                    auto vec = json[key];
                    return setValueFromJson(vec, "x", value.x) && setValueFromJson(vec, "y", value.y) && setValueFromJson(vec, "z", value.z);
                }
                catch (...) {
                    return false;
                }
                return true;
            }
            return false;
        }

        static bool setValueFromJson(ci::Json json, std::string key, Color& value) {
            if (json.contains(key)) {
                try {
                    auto vec = json[key];
                    return setValueFromJson(vec, "r", value.r) && setValueFromJson(vec, "g", value.g) && setValueFromJson(vec, "b", value.b);
                }
                catch (...) {
                    return false;
                }
                return true;
            }
            return false;
        }

        static bool setValueFromJson(ci::Json json, std::string key, glm::quat& value) {
            if (json.contains(key)) {
                try {
                    auto vec = json[key];
                    return setValueFromJson(vec, "x", value.x) && setValueFromJson(vec, "y", value.y) && setValueFromJson(vec, "z", value.z) && setValueFromJson(vec, "w", value.w);
                }
                catch (...) {
                    return false;
                }
                return true;
            }
            return false;
        }
	

		static ci::Json valueToJson(ci::vec2 vec) {
			ci::Json json = ci::Json::object();
			json["x"] = vec.x;
			json["y"] = vec.y;
			return json;
		}
		
        static ci::Json valueToJson(ci::vec3 vec) {
			ci::Json json = ci::Json::object();
			json["x"] = (float)vec.x;
			json["y"] = (float)vec.y;
			json["z"] = (float)vec.z;
			return json;
		}

        static ci::Json valueToJson(ci::Color color) {
            ci::Json json = ci::Json::object();
            json["r"] = color.r;
            json["g"] = color.g;
            json["b"] = color.b;
            return json;
        }

        static ci::Json valueToJson(glm::quat q) {
            ci::Json json = ci::Json::object();
            json["x"] = q.x;
            json["y"] = q.y;
            json["z"] = q.z;
            json["w"] = q.w;
            return json;
        }
	}
}