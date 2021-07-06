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
			class IBullet : public IMessage
			{
			public:
				virtual void CreateBulletTracer(const Vector& barrel, const Vector& start, const Vector& end, uint32_t numBullets, uint32_t iLarge, uint32_t numTracersVisible, float bulletSize) = 0;
				virtual void CreateBubbleTrail(const Vector& start, const Vector& end, uint32_t iLarge, float bulletSize) = 0;
			};
		}
	}
}
}
