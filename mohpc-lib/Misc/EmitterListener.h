#pragma once

#include <MOHPC/Script/Listener.h>

namespace MOHPC
{
	class EmitterListener : public Listener
	{
	private:
		struct Emitter* emitter;
		bool bInBlock;
		bool bProcessed;

	public:
		CLASS_PROTOTYPE(EmitterListener);

		EmitterListener();
		EmitterListener(Emitter* OutEmitter);

		bool FinishedParsing() const;

	private:
		void BeginOriginSpawn(Event* ev);
		void BeginOriginEmitter(Event* ev);
		void StartBlock(Event* ev);
		void EndBlock(Event* ev);
		void StartSFX(Event* ev);
		void StartSFXDelayed(Event* ev);
		void StartSFXInternal(Event* ev, bool bDelayed);
		void SetBaseAndAmplitude(Event* ev, Vector& Base, Vector& Amplitude);
		void SetSpawnRate(Event* ev);
		void SetScaleRate(Event* ev);
		void SetCount(Event* ev);
		void SetScale(Event* ev);
		void SetScaleUpDown(Event* ev);
		void SetScaleMin(Event* ev);
		void SetScaleMax(Event* ev);
		void SetModel(Event* ev);
		void SetOffset(Event* ev);
		void SetOffsetAlongAxis(Event* ev);
		void SetLife(Event* ev);
		void SetColor(Event* ev);
		void SetAlpha(Event* ev);
		void SetAngles(Event* ev);
		void SetRadialVelocity(Event* ev);
		void SetVelocity(Event* ev);
		void SetAngularVelocity(Event* ev);
		void SetRandomVelocity(Event* ev);
		void SetRandomVelocityAlongAxis(Event* ev);
		void SetAccel(Event* ev);
		void SetCone(Event* ev);
		void SetCircle(Event* ev);
		void SetSphere(Event* ev);
		void SetInwardSphere(Event* ev);
		void SetRadius(Event* ev);
		void SetFade(Event* ev);
		void SetFadeDelay(Event* ev);
		void SetFadeIn(Event* ev);
	};
}
