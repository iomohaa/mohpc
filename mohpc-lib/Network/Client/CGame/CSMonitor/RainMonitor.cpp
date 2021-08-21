#include <MOHPC/Network/Client/CGame/CSMonitor/RainMonitor.h>

using namespace MOHPC;
using namespace MOHPC::Network;
using namespace MOHPC::Network::CGame;

template<>
std::map<csNum_t, ConfigstringMonitorTemplate<RainMonitor>::Callback> ConfigstringMonitorTemplate<RainMonitor>::callbackMap
{
	{ CS::RAIN_DENSITY, &RainMonitor::setRainDensity },
	{ CS::RAIN_SPEED, &RainMonitor::setRainSpeed },
	{ CS::RAIN_SPEEDVARY, &RainMonitor::setRainSpeedVary },
	{ CS::RAIN_SLANT, &RainMonitor::setRainSlant },
	{ CS::RAIN_LENGTH, &RainMonitor::setRainLength },
	{ CS::RAIN_MINDIST, &RainMonitor::setRainMinDist },
	{ CS::RAIN_WIDTH, &RainMonitor::setRainWidth },
	{ CS::RAIN_SHADER, &RainMonitor::setRainShader },
	{ CS::RAIN_NUMSHADERS, &RainMonitor::setRainNumShaders }
};

MOHPC_OBJECT_DEFINITION(RainMonitor);

RainMonitor::RainMonitor(const SnapshotProcessorPtr& snapshotProcessor, const RainPtr& rainPtr)
	: ConfigstringMonitorTemplate(snapshotProcessor)
	, rain(rainPtr)
{
}

RainMonitor::~RainMonitor()
{
}

void RainMonitor::setRainDensity(const char* cs)
{
	rain->setDensity((float)atof(cs));
}


void RainMonitor::setRainSpeed(const char* cs)
{
	rain->setSpeed((float)atof(cs));
}

void RainMonitor::setRainSpeedVary(const char* cs)
{
	rain->setSpeedVariation(atoi(cs));
}

void RainMonitor::setRainSlant(const char* cs)
{
	rain->setSlant(atoi(cs));
}

void RainMonitor::setRainLength(const char* cs)
{
	rain->setLength((float)atof(cs));
}

void RainMonitor::setRainMinDist(const char* cs)
{
	rain->setMinimumDistance((float)atof(cs));
}

void RainMonitor::setRainWidth(const char* cs)
{
	rain->setWidth((float)atof(cs));
}

void RainMonitor::setRainShader(const char* cs)
{
	const uint32_t numShaders = rain->getNumShaders();
	if (numShaders)
	{
		for (uint32_t i = 0; i < numShaders; ++i)
		{
			// Multiple shaders
			rain->setShader((cs + std::to_string(i)).c_str(), i);
		}
	}
	else
	{
		// Single shader
		rain->setShader(cs, 0);
	}
}

void RainMonitor::setRainNumShaders(const char* cs)
{
	const str tmp = rain->getShader(0);
	const uint32_t numShaders = rain->getNumShaders();
	for (uint32_t i = 0; i < numShaders; ++i)
	{
		// Append shader number to the previous shader
		rain->setShader((tmp + std::to_string(i)).c_str(), i);
	}
}
