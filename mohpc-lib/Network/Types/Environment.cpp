#include <MOHPC/Network/Types/Environment.h>

using namespace MOHPC;
using namespace MOHPC::Network;

MOHPC_OBJECT_DEFINITION(environment_t);

environment_t::environment_t()
	: farplaneDistance(0.f)
	, skyAlpha(0.f)
	, skyboxFarplane(0.f)
	, skyboxSpeed(0.f)
	, farplaneBias(0.f)
	, farclipOverride(0.f)
	, farplaneCull(false)
	, skyPortal(false)
	, renderTerrain(true)
{
}

environment_t::~environment_t()
{
}

bool environment_t::isFarplaneCulling() const
{
	return farplaneCull;
}

float environment_t::getFarplane() const
{
	return farplaneDistance;
}

const_vec3p_t environment_t::getFarplaneColor() const
{
	return farplaneColor;
}

float environment_t::getSkyAlpha() const
{
	return skyAlpha;
}

bool environment_t::isSkyPortal() const
{
	return skyPortal;
}

float environment_t::getFarplaneBias() const
{
	return farplaneBias;
}

float environment_t::getSkyboxFarplane() const
{
	return skyboxFarplane;
}

float environment_t::getSkyboxSpeed() const
{
	return skyboxSpeed;
}

float environment_t::getFarclipOverride() const
{
	return farclipOverride;
}

const_vec3p_t environment_t::getFarplaneColorOverride() const
{
	return farplaneColorOverride;
}

bool environment_t::shouldRenderTerrain() const
{
	return renderTerrain;
}
