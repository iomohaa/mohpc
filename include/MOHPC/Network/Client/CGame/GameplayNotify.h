#pragma once

#include "../../NetGlobal.h"
#include "Messages/Bullet.h"
#include "Messages/Effect.h"
#include "Messages/HUD.h"
#include "Messages/Impact.h"
#include "Messages/Event.h"

namespace MOHPC
{
namespace Network
{
	namespace CGame
	{
		class GameplayNotify
		{
		public:
			MOHPC_NET_EXPORTS Messages::BulletImpl& getBulletNotify();
			MOHPC_NET_EXPORTS const Messages::BulletImpl& getBulletNotify() const;

			MOHPC_NET_EXPORTS Messages::EffectImpl& getEffectNotify();
			MOHPC_NET_EXPORTS const Messages::EffectImpl& getEffectNotify() const;

			MOHPC_NET_EXPORTS Messages::HUDImpl& getHUDNotify();
			MOHPC_NET_EXPORTS const Messages::HUDImpl& getHUDNotify() const;

			MOHPC_NET_EXPORTS Messages::ImpactImpl& getImpactNotify();
			MOHPC_NET_EXPORTS const Messages::ImpactImpl& getImpactNotify() const;

			MOHPC_NET_EXPORTS Messages::EventImpl& getEventNotify();
			MOHPC_NET_EXPORTS const Messages::EventImpl& getEventNotify() const;

		private:
			Messages::BulletImpl bulletNotify;
			Messages::EffectImpl effectNotify;
			Messages::HUDImpl HUDNotify;
			Messages::ImpactImpl impactNotify;
			Messages::EventImpl eventNotify;
		};
	}
}
}