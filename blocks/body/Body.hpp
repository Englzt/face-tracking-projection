/*
	Body Helper - 2024-25

	Do not distribute (including publishing, open-sourcing).
	Usage is only allowed in context of the practical course ("Medieninformatik Projekt", "KP Interaktives Theater") "EarlGrey" (SoSe25) at the Chair for Immersive Media Design (TU Dresden).

	-> ask lars.engeln@tu-dresden.de
*/

#pragma once
#include "cinder/app/App.h"
#include "BodyJoint.hpp"
#include "UniqueIDBase.hpp"
#include "cinder/gl/gl.h"

namespace act {
	namespace room {
		struct Body : public UniqueIDBase {

                  //create body joint for each BJT
			Body() {
				for (int i = 0; i < BJT_COUNT; i++)
					joints.push_back(BodyJoint::create((BodyJointType) i));
			};
			~Body() {}

			static std::shared_ptr<Body> create() { return std::make_shared<Body>(); };

			std::vector<BodyJointRef> joints;

			glm::vec3 getPosition() { return joints[BJT_SPINE_CHEST]->position; };

			//creates json object
			ci::Json toJson() {
				ci::Json json = ci::Json::object();
				json["uid"] = getUID();
				ci::Json jointsJson = ci::Json::array();
				for (auto&& joint : joints) {
					jointsJson.push_back(joint->toJson());
				}
				json["joints"] = jointsJson;
				return json;
			}

			bool fromJson(ci::Json json, bool override = false) {
				if (!json.contains("uid") || !json.contains("joints"))
					return false; 
				if (override)
					setUID(json["uid"]);
				else if (getUID() != json["uid"])
					return false;

				for (auto&& joint : json["joints"]) {
					int type = -1;
					util::setValueFromJson(joint, "type", type);
					joints[type]->fromJson(joint);
				}
				return true;
			}

			void draw(bool debug = false) {
				gl::pushMatrices();
				gl::drawCube(ci::vec3(0.0f), ci::vec3(0.1f, 0.075f, 0.075f));
				auto color = ColorA(0.9f, 0.9f, 0.9f, 0.85f);
				float sphereRadius = 0.055f;

				for (auto&& joint : joints)
				{
					vec3 pos = joint->position;
					switch (joint->confidenceLevel) {
					case act::room::BJC_NONE:
						gl::color(ColorA(color.r, color.g, color.b, 0.4f));
						gl::lineWidth(2);
						break;
					case act::room::BJC_LOW:
						gl::color(ColorA(color.r, color.g, color.b, 0.6f));
						gl::lineWidth(4);
						break;
					case act::room::BJC_HIGH:
						gl::color(color);
						gl::lineWidth(4);
						break;
					default:
						break;
					}

					if (joint->type == act::room::BJT_HEAD)
						gl::drawSphere(pos, sphereRadius * 4.0f / 3.0f, 8);
					else
						gl::drawSphere(pos, sphereRadius, 8);

					vec3 parent = joints[act::room::bodyJointParentLookUp[joint->type]]->position;

					gl::drawLine(pos, parent);

					// drawVector only if debugging
					if (debug) {
						gl::pushMatrices();
						gl::translate(pos);
						gl::rotate(joint->orientation);
						gl::drawCoordinateFrame(0.2f);
						gl::popMatrices();
					}
				}

				gl::lineWidth(1);
				gl::popMatrices();
			}

		};	using BodyRef = std::shared_ptr<Body>;
			using BodyRefList = std::vector<BodyRef>;
	}
}