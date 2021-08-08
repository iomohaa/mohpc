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
			class IBullet : public IMessage
			{
			public:
				virtual void CreateBulletTracer(const vec3r_t barrel, const vec3r_t start, const vec3r_t end, uint32_t numBullets, uint32_t iLarge, uint32_t numTracersVisible, float bulletSize) = 0;
				virtual void CreateBubbleTrail(const vec3r_t start, const vec3r_t end, uint32_t iLarge, float bulletSize) = 0;
			};
		}
	}
}
}
