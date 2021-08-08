#pragma once

#include <MOHPC/Assets/Managers/AssetManager.h>
#include <MOHPC/Common/SimpleVector.h>
#include <morfuse/Script/Listener.h>

namespace MOHPC
{
	struct Emitter;

	class EmitterListener : public mfuse::Listener
	{
		MFUS_CLASS_PROTOTYPE(EmitterListener);

	public:
		EmitterListener();
		EmitterListener(Emitter* OutEmitter);

		void SetAssetManager(const AssetManagerPtr& inAssetManager);
		bool FinishedParsing() const;

	private:
		void BeginOriginSpawn(mfuse::Event* ev);
		void BeginOriginEmitter(mfuse::Event* ev);
		void StartBlock(mfuse::Event* ev);
		void EndBlock(mfuse::Event* ev);
		void StartSFX(mfuse::Event* ev);
		void StartSFXDelayed(mfuse::Event* ev);
		void StartSFXInternal(mfuse::Event* ev, bool bDelayed);
		void SetBaseAndAmplitude(mfuse::Event* ev, vec3r_t Base, vec3r_t Amplitude);
		void SetSpawnRate(mfuse::Event* ev);
		void SetScaleRate(mfuse::Event* ev);
		void SetCount(mfuse::Event* ev);
		void SetScale(mfuse::Event* ev);
		void SetScaleUpDown(mfuse::Event* ev);
		void SetScaleMin(mfuse::Event* ev);
		void SetScaleMax(mfuse::Event* ev);
		void SetModel(mfuse::Event* ev);
		void SetOffset(mfuse::Event* ev);
		void SetOffsetAlongAxis(mfuse::Event* ev);
		void SetLife(mfuse::Event* ev);
		void SetColor(mfuse::Event* ev);
		void SetAlpha(mfuse::Event* ev);
		void SetAngles(mfuse::Event* ev);
		void SetRadialVelocity(mfuse::Event* ev);
		void SetVelocity(mfuse::Event* ev);
		void SetAngularVelocity(mfuse::Event* ev);
		void SetRandomVelocity(mfuse::Event* ev);
		void SetRandomVelocityAlongAxis(mfuse::Event* ev);
		void SetAccel(mfuse::Event* ev);
		void SetCone(mfuse::Event* ev);
		void SetCircle(mfuse::Event* ev);
		void SetSphere(mfuse::Event* ev);
		void SetInwardSphere(mfuse::Event* ev);
		void SetRadius(mfuse::Event* ev);
		void SetFade(mfuse::Event* ev);
		void SetFadeDelay(mfuse::Event* ev);
		void SetFadeIn(mfuse::Event* ev);

	private:
		Emitter* emitter;
		bool bInBlock;
		bool bProcessed;
		AssetManagerPtr assetManager;
	};
}
