#include <MOHPC/Network/Client/CGame/Messages/Effect.h>

using namespace MOHPC;
using namespace MOHPC::Network;
using namespace MOHPC::Network::CGame;
using namespace MOHPC::Network::CGame::Messages;

void Messages::EffectImpl::SpawnEffect(const Vector& origin, const Vector& normal, const char* modelName)
{
	spawnEffectHandler.broadcast(origin, normal, modelName);
}

void Messages::EffectImpl::SpawnDebris(debrisType_e debrisType, const Vector& origin, uint32_t numDebris)
{
	spawnDebrisHandler.broadcast(debrisType, origin, numDebris);
}
