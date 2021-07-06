#include <MOHPC/Network/Client/CGame/GameplayNotify.h>

using namespace MOHPC;
using namespace MOHPC::Network;
using namespace MOHPC::Network::CGame;

Messages::BulletImpl& GameplayNotify::getBulletNotify()
{
	return bulletNotify;
}

const Messages::BulletImpl& GameplayNotify::getBulletNotify() const
{
	return bulletNotify;
}

Messages::EffectImpl& GameplayNotify::getEffectNotify()
{
	return effectNotify;
}

const Messages::EffectImpl& GameplayNotify::getEffectNotify() const
{
	return effectNotify;
}

Messages::HUDImpl& GameplayNotify::getHUDNotify()
{
	return HUDNotify;
}

const Messages::HUDImpl& GameplayNotify::getHUDNotify() const
{
	return HUDNotify;
}

Messages::ImpactImpl& GameplayNotify::getImpactNotify()
{
	return impactNotify;
}

const Messages::ImpactImpl& GameplayNotify::getImpactNotify() const
{
	return impactNotify;
}

Messages::EventImpl& GameplayNotify::getEventNotify()
{
	return eventNotify;
}

const Messages::EventImpl& GameplayNotify::getEventNotify() const
{
	return eventNotify;
}
