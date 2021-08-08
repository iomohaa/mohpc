#pragma once

#include "Base.h"
#include "../../../../Common/SimpleVector.h"

#include <cstdint>

namespace MOHPC
{
namespace Network
{
	namespace CGame
	{
		namespace Messages
		{
			class IEvent : public IMessage
			{
			public:
				virtual void Hit() = 0;
				virtual void GotKill() = 0;
				virtual void PlayVoice(const vec3r_t origin, bool local, uint8_t clientNum, const char* soundName) = 0;
			};
		}
	}
}
}
