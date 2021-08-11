#include <MOHPC/Network/Client/CGame/Rain.h>

using namespace MOHPC;
using namespace MOHPC::Network;
using namespace MOHPC::Network::CGame;

MOHPC_OBJECT_DEFINITION(Rain);

Rain::Rain()
	: density(0.f)
	, speed(0.f)
	, speedVary(0)
	, slant(0)
	, length(0.f)
	, minDist(0.f)
	, width(0.f)
	, numShaders(0)
{
}

Rain::~Rain()
{
}

void Rain::setDensity(float densityVal)
{
	density = densityVal;
}

float Rain::getDensity() const
{
	return density;
}

void Rain::setSpeed(float speedVal)
{
	speed = speedVal;
}

float Rain::getSpeed() const
{
	return speed;
}

void Rain::setLength(float lengthVal)
{
	length = lengthVal;
}

float Rain::getLength() const
{
	return length;
}

void Rain::setMinimumDistance(float distanceVal)
{
	minDist = distanceVal;
}

float Rain::getMinimumDistance() const
{
	return minDist;
}

void Rain::setWidth(float widthVal)
{
	width = widthVal;
}

float Rain::getWidth() const
{
	return width;
}

void Rain::setSpeedVariation(uint32_t variationVal)
{
	speedVary = variationVal;
}

uint32_t Rain::getSpeedVariation() const
{
	return speedVary;
}

void Rain::setSlant(uint32_t slantVal)
{
	slant = slantVal;
}

uint32_t Rain::getSlant() const
{
	return slant;
}

void Rain::addShader(const char* shaderVal)
{
	if (numShaders < MAX_RAIN_SHADERS)
	{
		// can add the shader
		shader[numShaders++] = shaderVal;
	}
}

void Rain::setNumShader(uint32_t num)
{
	numShaders = num;
}

uint32_t Rain::getNumShaders() const
{
	return numShaders;
}

void Rain::setShader(const char* shaderVal, uint32_t index)
{
	shader[index] = shaderVal;
}

const char* Rain::getShader(uint8_t index) const
{
	if (index < MAX_RAIN_SHADERS) {
		return shader[index].c_str();
	}
	else {
		return nullptr;
	}
}
