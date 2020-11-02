#pragma once

#include "../Global.h"
#include "../Asset.h"
#include "../Math.h"
#include "../Vector.h"
#include "../Managers/ShaderManager.h"
#include "../Utilities/SharedPtr.h"

namespace MOHPC
{
	struct DCLMarkDef
	{
		/** Shader used for this decal. */
		ShaderPtr shader;

		/** Location of the decal. */
		Vector position;

		/** The direction of the decal. */
		Vector projection;

		/** The radius at which the decal will be applied on walls. */
		float radius;

		/** Height scale of the decal. */
		float heightScale;

		/** Width scale of the decal. */
		float widthScale;

		/** Rotation (yaw of the dir) of the decal. */
		float rotation;

		/** The RGBA color of the decal.*/
		vec4_t color;

		/** True to apply lighting on the decal, false otherwise. */
		bool bDoLighting;
	};

	class DCL : public Asset
	{
		CLASS_BODY(DCL);

	private:
		DCLMarkDef* dclDecals;
		size_t numDecals;
		size_t numFragments;

	public:
		MOHPC_EXPORTS DCL();
		MOHPC_EXPORTS ~DCL();

		virtual bool Load() override;

		/** Get the number of decals. */
		MOHPC_EXPORTS size_t GetNumDecals() const;

		/** Get a decal definition at the specified index. */
		MOHPC_EXPORTS const DCLMarkDef* GetDecal(size_t index) const;
	};
	typedef SharedPtr<DCL> DCLPtr;

	namespace DCLError
	{
		class Base : public std::exception {};

		class BadHeader : public Base
		{
		public:
			BadHeader(const uint8_t foundHeader[4]);

			MOHPC_EXPORTS const uint8_t* getHeader() const;

		private:
			uint8_t foundHeader[4];
		};

		class WrongVersion : public Base
		{
		public:
			WrongVersion(uint32_t inVersion);

			MOHPC_EXPORTS uint32_t getVersion() const;

		private:
			uint32_t version;
		};
	}
}
