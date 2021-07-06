#pragma once

#include "Base.h"
#include "../Effect.h"

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
			class IEffect : public IMessage
			{
			public:
				virtual void SpawnEffect(const Vector& origin, const Vector& normal, const char* modelName) = 0;
				virtual void SpawnDebris(debrisType_e debrisType, const Vector& origin, uint32_t numDebris) = 0;
			};
		}
	}
}
}
