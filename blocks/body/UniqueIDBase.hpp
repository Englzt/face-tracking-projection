/*
	Body Helper - 2024-25

	Do not distribute (including publishing, open-sourcing).
	Usage is only allowed in context of the practical course ("Medieninformatik Projekt", "KP Interaktives Theater") "EarlGrey" (SoSe25) at the Chair for Immersive Media Design (TU Dresden).

	-> ask lars.engeln@tu-dresden.de
*/

#pragma once
#include "cinder/app/App.h"
#include <chrono>


using namespace ci;
using namespace ci::app;

namespace act {

		using UID = std::string;

		class UniqueIDBase
		{
		public:
			UniqueIDBase() { m_uid = createUID(); };
			virtual ~UniqueIDBase() {};

			UID getUID() { return m_uid; }
			void setUID(UID uid) { if(!uid.empty()) m_uid = uid; }

		protected:
			UID m_uid;

		private:
			UID createUID() {
				auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock().now().time_since_epoch()).count();
				std::stringstream strstr;
				strstr << ms;
				std::string msStr = strstr.str();
				strstr.str("");

				int half  = (int)(msStr.length() * 0.5);
				auto part1 = msStr.substr(0, half);
				auto part2 = msStr.substr(half, msStr.length() - half);
				
				int salt = ((int)(((double)rand() / (RAND_MAX)) * 3839) + 256);
				strstr << std::hex << stoll(part1) << "-" << std::hex << stoll(part2) << std::hex << salt;

				return strstr.str();
			};
		};

}