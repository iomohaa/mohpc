#pragma once

#include "../Global.h"
#include "Manager.h"
#include "../Vector.h"
#include <vector>
#include <memory>

namespace MOHPC
{
	struct MOHPC_EXPORTS Sprite
	{
		enum spriteType_e
		{
			ST_None,
			ST_Tiki,
			ST_Shader,
		};

		spriteType_e spriteType;
		union {
			std::shared_ptr<class TIKI>* Tiki;
			class ShaderRef* Shader;
			void* pointerValue;
		};

		Sprite();
		Sprite(const Sprite& sprite);
		Sprite(Sprite&& sprite);
		~Sprite();
		Sprite& operator=(const Sprite& sprite);
	};

	struct Emitter
	{
		friend class EmitterManager;

	public:
		enum emitterFlags_e
		{
			EF_Fade = (1<<0),
			EF_FadeIn = (1<<1),
			EF_Align = (1<< 2),
			EF_AlignOnce = (1<<3),
			EF_HardLink = (1<<4),
			EF_ParentLink = (1<<5),
			EF_UseParentAxis = (1<<6),
			EF_UpDownScale = (1<<7),
		};

		enum emitterSpawnType_e
		{
			EST_None,
			EST_Sphere,
			EST_Cone,
			EST_InwardSphere,
			EST_Circle
		};

		std::string emitterName;
		bool bIsEmitter;
		int32_t flags;
		emitterSpawnType_e spawnType;
		Sprite sprite;
		Vector accel;
		Vector radialVelocity;
		Vector originOffsetBase;
		Vector originOffsetAmplitude;
		Vector anglesOffsetBase;
		Vector anglesOffsetAmplitude;
		Vector randVelocityBase;
		Vector randVelocityAmplitude;
		Vector randAVelocityBase;
		Vector randAVelocityAmplitude;
		int32_t count;
		float life;
		float randomLife;
		Vector color;
		float alpha;
		float scaleRate;
		float scaleMin;
		float scaleMax;
		float scale;
		float fadeInTime;
		float fadeDelay;
		float startTime;
		float sphereRadius;
		float coneHeight;
		float spawnRate;
		float forwardVelocity;
		Vector dlight;
		float lightRadius;

		MOHPC_EXPORTS Emitter();

		MOHPC_EXPORTS const char* GetEmitterName() const;
	};

	struct EmitterResults
	{
		std::vector<Emitter> Emitters;
		int32_t animNum;
		std::string animName;

		MOHPC_EXPORTS EmitterResults();

		MOHPC_EXPORTS size_t GetNumEmitters() const;
		MOHPC_EXPORTS const Emitter* GetEmitterAt(size_t Index) const;
		MOHPC_EXPORTS const char* GetAnimName() const;
	};

	class TIKI;

	class EmitterManager : public Manager
	{
		CLASS_BODY(EmitterManager);

	public:
		MOHPC_EXPORTS EmitterManager();

		MOHPC_EXPORTS bool ParseEmitters(const TIKI* Tiki, EmitterResults& Results);

	private:
		bool ProcessCommand(const std::vector<std::string>& Arguments, class EmitterListener& Listener);
	};
}
