#pragma once

#include "Base.h"
#include "../../../../Common/SimpleVector.h"

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
				virtual void Impact(const vec3r_t origin, const vec3r_t normal, uint32_t large) = 0;
				virtual void MeleeImpact(const vec3r_t origin, const vec3r_t normal) = 0;
				virtual void Explosion(const vec3r_t origin, const char* modelName) = 0;
			};
		}
	}
}
}
