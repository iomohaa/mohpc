#pragma once

#include "../../NetGlobal.h"
#include "../../NetObject.h"
#include "../../../Utility/SharedPtr.h"
#include "../../../Common/str.h"

#include <cstdint>

namespace MOHPC
{
namespace Network
{
namespace CGame
{
	static constexpr unsigned long MAX_RAIN_SHADERS = 16;

	class Rain
	{
		MOHPC_NET_OBJECT_DECLARATION(Rain);

	public:
		MOHPC_NET_EXPORTS Rain();
		MOHPC_NET_EXPORTS ~Rain();

		MOHPC_NET_EXPORTS void setDensity(float density);
		MOHPC_NET_EXPORTS float getDensity() const;

		MOHPC_NET_EXPORTS void setSpeed(float speed);
		MOHPC_NET_EXPORTS float getSpeed() const;
		MOHPC_NET_EXPORTS void setSpeedVariation(uint32_t variation);
		MOHPC_NET_EXPORTS uint32_t getSpeedVariation() const;
		MOHPC_NET_EXPORTS void setSlant(uint32_t slant);
		MOHPC_NET_EXPORTS uint32_t getSlant() const;

		MOHPC_NET_EXPORTS void setLength(float length);
		MOHPC_NET_EXPORTS float getLength() const;
		MOHPC_NET_EXPORTS void setMinimumDistance(float distance);
		MOHPC_NET_EXPORTS float getMinimumDistance() const;
		MOHPC_NET_EXPORTS void setWidth(float width);
		MOHPC_NET_EXPORTS float getWidth() const;

		MOHPC_NET_EXPORTS void addShader(const char* shader);
		MOHPC_NET_EXPORTS void setNumShader(uint32_t num);
		MOHPC_NET_EXPORTS uint32_t getNumShaders() const;
		MOHPC_NET_EXPORTS void setShader(const char* shader, uint32_t index);
		MOHPC_NET_EXPORTS const char* getShader(uint8_t index) const;

	private:
		float density;
		float speed;
		float length;
		float minDist;
		float width;
		uint32_t speedVary;
		uint32_t slant;
		uint32_t numShaders;
		str shader[MAX_RAIN_SHADERS];
	};
	using RainPtr = SharedPtr<Rain>;
}
}
}