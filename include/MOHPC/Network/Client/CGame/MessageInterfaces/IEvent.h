#pragma once

#include "Base.h"

#include <cstdint>

namespace MOHPC
{
class Vector;

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
				virtual void PlayVoice(const Vector& origin, bool local, uint8_t clientNum, const char* soundName) = 0;
			};
		}
	}
}
}
