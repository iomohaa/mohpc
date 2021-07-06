#pragma once

#include "Base.h"
#include "../MessageInterfaces/IBullet.h"

namespace MOHPC
{
namespace Network
{
	namespace CGame
	{
		namespace Handlers
		{
			//=== FX functions

			/**
			 * Callback for creating bullet tracers.
			 *
			 * @param	barrel			tag_barrel where the bullet has been emitted from.
			 * @param	start			Start of the bullet tracer.
			 * @param	end				Where the bullet tracer should end.
			 * @param	numBullets		Number of bullets that have been fired at once (could be fired from a shotgun).
			 * @param	iLarge			Whether or not it's a large bullet.
			 * @param	tracerVisible	Specify if this tracer is visible.
			 * @param	bulletSize		The length of the bullet.
			 */
			struct CreateBulletTracer : public HandlerNotifyBase<void(const Vector& barrel, const Vector& start, const Vector& end, uint32_t numBullets, uint32_t iLarge, uint32_t numTracersVisible, float bulletSize)> {};

			/**
			 * Underwater bubble trail.
			 *
			 * @param	start		Trail's start.
			 * @param	end			Trail's end.
			 * @param	iLarge		If it's a large trail.
			 * @param	bulletSize	Length of the trail.
			 */
			struct CreateBubbleTrail : public HandlerNotifyBase<void(const Vector& start, const Vector& end, uint32_t iLarge, float bulletSize)> {};
		}

		namespace Messages
		{
			class BulletImpl : public IBullet
			{
			public:
				void CreateBulletTracer(const Vector& barrel, const Vector& start, const Vector& end, uint32_t numBullets, uint32_t iLarge, uint32_t numTracersVisible, float bulletSize) override;
				void CreateBubbleTrail(const Vector& start, const Vector& end, uint32_t iLarge, float bulletSize) override;

			public:
				FunctionList<Handlers::CreateBulletTracer> createBulletTracerHandler;
				FunctionList<Handlers::CreateBubbleTrail> createBubbleTrailHandler;
			};
		}
	}
}
}
