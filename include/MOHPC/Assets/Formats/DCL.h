#pragma once

#include "../Asset.h"
#include "../Managers/ShaderManager.h"
#include "../../Common/Math.h"
#include "../../Common/Vector.h"
#include "../../Utility/SharedPtr.h"

namespace MOHPC
{
	struct DCLMarkDef
	{
		/** Shader used for this decal. */
		ShaderPtr shader;

		/** Location of the decal. */
		vec3_t position;

		/** The direction of the decal. */
		vec3_t projection;

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
		MOHPC_ASSET_OBJECT_DECLARATION(DCL);

	private:
		DCLMarkDef* dclDecals;
		size_t numDecals;
		size_t numFragments;

	public:
		MOHPC_ASSETS_EXPORTS DCL();
		MOHPC_ASSETS_EXPORTS ~DCL();

		virtual void Load() override;

		/** Get the number of decals. */
		MOHPC_ASSETS_EXPORTS size_t GetNumDecals() const;

		/** Get a decal definition at the specified index. */
		MOHPC_ASSETS_EXPORTS const DCLMarkDef* GetDecal(size_t index) const;
	};
	typedef SharedPtr<DCL> DCLPtr;

	namespace DCLError
	{
		class Base : public std::exception {};

		class BadHeader : public Base
		{
		public:
			BadHeader(const uint8_t foundHeader[4]);

			MOHPC_ASSETS_EXPORTS const uint8_t* getHeader() const;

		public:
			const char* what() const noexcept override;

		private:
			uint8_t foundHeader[4];
		};

		class WrongVersion : public Base
		{
		public:
			WrongVersion(uint32_t inVersion);

			MOHPC_ASSETS_EXPORTS uint32_t getVersion() const;

		public:
			const char* what() const noexcept override;

		private:
			uint32_t version;
		};
	}
}
