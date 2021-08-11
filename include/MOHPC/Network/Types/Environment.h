#pragma once

#include "../NetGlobal.h"
#include "../NetObject.h"
#include "../../Common/Vector.h"
#include "../../Utility/SharedPtr.h"

namespace MOHPC
{
namespace Network
{
	struct environment_t
	{
		MOHPC_NET_OBJECT_DECLARATION(environment_t);

	public:
		MOHPC_NET_EXPORTS environment_t();
		MOHPC_NET_EXPORTS ~environment_t();

		/** If the farplane culls distant objects. */
		MOHPC_NET_EXPORTS bool isFarplaneCulling() const;

		/** The max distance of the fog. */
		MOHPC_NET_EXPORTS float getFarplane() const;

		/** Fog color. */
		MOHPC_NET_EXPORTS const_vec3p_t getFarplaneColor() const;

		/** The fog bias. */
		MOHPC_NET_EXPORTS float getFarplaneBias() const;

		/** The farplane in the skybox. */
		MOHPC_NET_EXPORTS float getSkyboxFarplane() const;

		/** The skybox movement speed. */
		MOHPC_NET_EXPORTS float getSkyboxSpeed() const;

		/** The farclip override. */
		MOHPC_NET_EXPORTS float getFarclipOverride() const;

		/** Colors for temporarily overriding fog. */
		MOHPC_NET_EXPORTS const_vec3p_t getFarplaneColorOverride() const;

		/** True if terrain should be rendered. */
		MOHPC_NET_EXPORTS bool shouldRenderTerrain() const;

		/** Sky alpha. */
		MOHPC_NET_EXPORTS float getSkyAlpha() const;

		/** True if this is a sky portal. */
		MOHPC_NET_EXPORTS bool isSkyPortal() const;

	public:
		vec3_t farplaneColor;
		vec3_t farplaneColorOverride;
		float farplaneDistance;
		float skyAlpha;
		float skyboxFarplane;
		float skyboxSpeed;
		float farplaneBias;
		float farclipOverride;
		bool farplaneCull : 1;
		bool skyPortal : 1;
		bool renderTerrain : 1;
	};
	using EnvironmentPtr = SharedPtr<environment_t>;
}
}