#pragma once

#include "../UtilityGlobal.h"
#include "../UtilityObject.h"
#include "../../Common/Vector.h"
#include "../../Common/str.h"
#include "../../Common/SimpleVector.h"
#include "../../Assets/Managers/Manager.h"
#include "../SharedPtr.h"

#include <vector>

namespace MOHPC
{
	struct MOHPC_UTILITY_EXPORTS Sprite
	{
		enum spriteType_e
		{
			ST_None,
			ST_Tiki,
			ST_Shader,
		};

		spriteType_e spriteType;
		union {
			SharedPtr<class TIKI>* Tiki;
			SharedPtr<class Shader>* Shader;
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

		str emitterName;
		bool bIsEmitter;
		int32_t flags;
		emitterSpawnType_e spawnType;
		Sprite sprite;
		vec3_t accel;
		vec3_t radialVelocity;
		vec3_t originOffsetBase;
		vec3_t originOffsetAmplitude;
		vec3_t anglesOffsetBase;
		vec3_t anglesOffsetAmplitude;
		vec3_t randVelocityBase;
		vec3_t randVelocityAmplitude;
		vec3_t randAVelocityBase;
		vec3_t randAVelocityAmplitude;
		int32_t count;
		float life;
		float randomLife;
		vec3_t color;
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
		vec3_t dlight;
		float lightRadius;

		MOHPC_UTILITY_EXPORTS Emitter();

		MOHPC_UTILITY_EXPORTS const char* GetEmitterName() const;
	};

	struct EmitterResults
	{
		std::vector<Emitter> Emitters;
		int32_t animNum;
		str animName;

		MOHPC_UTILITY_EXPORTS EmitterResults();

		MOHPC_UTILITY_EXPORTS size_t GetNumEmitters() const;
		MOHPC_UTILITY_EXPORTS const Emitter* GetEmitterAt(size_t Index) const;
		MOHPC_UTILITY_EXPORTS const char* GetAnimName() const;
	};

	class TIKI;

	class EmitterManager : public Manager
	{
		MOHPC_UTILITY_OBJECT_DECLARATION(EmitterManager);

	public:
		MOHPC_UTILITY_EXPORTS EmitterManager();

		MOHPC_UTILITY_EXPORTS bool ParseEmitters(const TIKI* Tiki, EmitterResults& Results);

	private:
		bool ProcessCommand(const std::vector<str>& Arguments, class EmitterListener& Listener);
	};
	using EmitterManagerPtr = SharedPtr<EmitterManager>;
}
