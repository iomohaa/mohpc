#include <MOHPC/Network/Client/CGame/Messages/Effect.h>

using namespace MOHPC;
using namespace MOHPC::Network;
using namespace MOHPC::Network::CGame;
using namespace MOHPC::Network::CGame::Messages;

void Messages::EffectImpl::SpawnEffect(const vec3r_t origin, const vec3r_t normal, const char* modelName)
{
	spawnEffectHandler.broadcast(origin, normal, modelName);
}

void Messages::EffectImpl::SpawnDebris(debrisType_e debrisType, const vec3r_t origin, uint32_t numDebris)
{
	spawnDebrisHandler.broadcast(debrisType, origin, numDebris);
}
