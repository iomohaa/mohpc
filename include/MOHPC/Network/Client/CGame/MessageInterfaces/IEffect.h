#pragma once

#include "Base.h"
#include "../Effect.h"
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
			class IEffect : public IMessage
			{
			public:
				virtual void SpawnEffect(const vec3r_t origin, const vec3r_t normal, const char* modelName) = 0;
				virtual void SpawnDebris(debrisType_e debrisType, const vec3r_t origin, uint32_t numDebris) = 0;
			};
		}
	}
}
}
