#include <MOHPC/Network/Client/CGame/Messages/Impact.h>

using namespace MOHPC;
using namespace MOHPC::Network;
using namespace MOHPC::Network::CGame;
using namespace MOHPC::Network::CGame::Messages;

void ImpactImpl::Impact(const vec3r_t origin, const vec3r_t normal, uint32_t large)
{
	impactHandler.broadcast(origin, normal, large);
}

void ImpactImpl::MeleeImpact(const vec3r_t origin, const vec3r_t normal)
{
	meleeImpactHandler.broadcast(origin, normal);
}

void ImpactImpl::Explosion(const vec3r_t origin, const char* modelName)
{
	explosionHandler.broadcast(origin, modelName);
}
