#pragma once

#include "Base.h"
#include "../MessageInterfaces/IImpact.h"

namespace MOHPC
{
namespace Network
{
	namespace CGame
	{
		namespace Handlers
		{
			/**
			 * Called on impact.
			 *
			 * @param	origin	Position for melee impact.
			 * @param	normal	Direction of the impact.
			 * @param	large	If it's a large impact.
			 */
			struct Impact : public HandlerNotifyBase<void(const Vector& origin, const Vector& normal, uint32_t large)> {};

			/**
			 * Called on melee impact.
			 *
			 * @param	start	Start position for melee impact.
			 * @param	end		End position for melee impact.
			 */
			struct MeleeImpact : public HandlerNotifyBase<void(const Vector& start, const Vector& end)> {};

			/**
			 * Called when an explosion occurs.
			 *
			 * @param	origin	Location of the explosion.
			 * @param	type	Explosion type.
			 */
			struct MakeExplosionEffect : public HandlerNotifyBase<void(const Vector& origin, const char* modelName)> {};
		}

		namespace Messages
		{
			class ImpactImpl : public IImpact
			{
			public:
				void Impact(const Vector& origin, const Vector& normal, uint32_t large) override;
				void MeleeImpact(const Vector& origin, const Vector& normal) override;
				void Explosion(const Vector& origin, const char* modelName) override;

			public:
				FunctionList<Handlers::Impact> impactHandler;
				FunctionList<Handlers::MeleeImpact> meleeImpactHandler;
				FunctionList<Handlers::MakeExplosionEffect> explosionHandler;
			};
		}
	}
}
}
