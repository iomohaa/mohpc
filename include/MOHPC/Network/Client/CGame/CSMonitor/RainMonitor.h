#pragma once

#include "ConfigStringMonitor.h"
#include "../Rain.h"
#include "../../../../Utility/SharedPtr.h"

namespace MOHPC
{
namespace Network
{
namespace CGame
{
	class RainMonitor : public ConfigstringMonitorTemplate<RainMonitor>
	{
		MOHPC_NET_OBJECT_DECLARATION(RainMonitor);

		friend ConfigstringMonitorTemplate;

	public:
		MOHPC_NET_EXPORTS RainMonitor(const SnapshotProcessorPtr& snapshotProcessor, const RainPtr& rain);
		MOHPC_NET_EXPORTS ~RainMonitor();

		//void configStringModified(csNum_t num, const char* cs) override;

	private:
		void setRainDensity(const char* cs);
		void setRainSpeed(const char* cs);
		void setRainSpeedVary(const char* cs);
		void setRainSlant(const char* cs);
		void setRainLength(const char* cs);
		void setRainMinDist(const char* cs);
		void setRainWidth(const char* cs);
		void setRainShader(const char* cs);
		void setRainNumShaders(const char* cs);

	private:
		RainPtr rain;
	};
	using RainMonitorPtr = SharedPtr<RainMonitor>;
}
}
}
