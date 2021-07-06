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
			class IImpact : public IMessage
			{
			public:
				virtual void Impact(const Vector& origin, const Vector& normal, uint32_t large) = 0;
				virtual void MeleeImpact(const Vector& origin, const Vector& normal) = 0;
				virtual void Explosion(const Vector& origin, const char* modelName) = 0;
			};
		}
	}
}
}
