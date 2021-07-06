#pragma once

#include "Base.h"
#include "../MessageInterfaces/IEffect.h"

namespace MOHPC
{
namespace Network
{
	namespace CGame
	{
		namespace Handlers
		{
			/**
			 * Called to spawn an effect.
			 *
			 * @param	origin	Location of the effect.
			 * @param	normal	Explosion's normal direction.
			 * @param	type	Type of the effect.
			 */
			struct SpawnEffect : public HandlerNotifyBase<void(const Vector& origin, const Vector& normal, const char* modelName)> {};

			/**
			 * Spawn a debris of the specified type.
			 *
			 * @param	debrisType	Type of the debris.
			 * @param	origin		Location where to spawn the debris.
			 * @param	numDebris	Number of debris to spawn.
			 */
			struct SpawnDebris : public HandlerNotifyBase<void(debrisType_e debrisType, const Vector& origin, uint32_t numDebris)> {};
		}

		namespace Messages
		{
			class EffectImpl : public IEffect
			{
			public:
				void SpawnEffect(const Vector& origin, const Vector& normal, const char* modelName) override;
				void SpawnDebris(debrisType_e debrisType, const Vector& origin, uint32_t numDebris) override;

			public:
				FunctionList<Handlers::SpawnEffect> spawnEffectHandler;
				FunctionList<Handlers::SpawnDebris> spawnDebrisHandler;
			};
		}
	}
}
}
