#include <MOHPC/Network/Client/CGame/Messages/Event.h>

using namespace MOHPC;
using namespace MOHPC::Network;
using namespace MOHPC::Network::CGame;
using namespace MOHPC::Network::CGame::Messages;

void EventImpl::Hit()
{
	hitHandler.broadcast();
}

void EventImpl::GotKill()
{
	gotKillHandler.broadcast();
}

void EventImpl::PlayVoice(const vec3r_t origin, bool local, uint8_t clientNum, const char* soundName)
{
	voiceMessageHandler.broadcast(origin, local, clientNum, soundName);
}
