#pragma once

#include "../../NetGlobal.h"
#include "../../ProtocolSingleton.h"

namespace MOHPC
{
namespace Network
{
	namespace CGame
	{
		enum class debrisType_e : unsigned char { crate, window };

		/**
		 * This is the list of effects.
		 *
		 * To get the full model path, use getEffectName().
		 */
		enum class effects_e
		{
			barrel_oil_leak_big = 0,
			barrel_oil_leak_medium,
			barrel_oil_leak_small,
			barrel_oil_leak_splat,
			barrel_water_leak_big,
			barrel_water_leak_medium,
			barrel_water_leak_small,
			barrel_water_leak_splat,
			bazookaexp_base,
			bazookaexp_dirt,
			bazookaexp_snow,
			bazookaexp_stone,
			bh_carpet_hard,
			bh_carpet_lite,
			bh_dirt_hard,
			bh_dirt_lite,
			bh_foliage_hard,
			bh_foliage_lite,
			bh_glass_hard,
			bh_glass_lite,
			bh_grass_hard,
			bh_grass_lite,
			bh_human_uniform_hard,
			bh_human_uniform_lite,
			bh_metal_hard,
			bh_metal_lite,
			bh_mud_hard,
			bh_mud_lite,
			bh_paper_hard,
			bh_paper_lite,
			bh_sand_hard,
			bh_sand_lite,
			bh_snow_hard,
			bh_snow_lite,
			bh_stone_hard,
			bh_stone_lite,
			bh_water_hard,
			bh_water_lite,
			bh_wood_hard,
			bh_wood_lite,
			fs_dirt,
			fs_grass,
			fs_heavy_dust,
			fs_light_dust,
			fs_mud,
			fs_puddle,
			fs_sand,
			fs_snow,
			fx_fence_wood,
			grenexp_base,
			grenexp_carpet,
			grenexp_dirt,
			grenexp_foliage,
			grenexp_grass,
			grenexp_gravel,
			grenexp_metal,
			grenexp_mud,
			grenexp_paper,
			grenexp_sand,
			grenexp_snow,
			grenexp_stone,
			grenexp_water,
			grenexp_wood,
			heavyshellexp_base,
			heavyshellexp_dirt,
			heavyshellexp_snow,
			heavyshellexp_stone,
			tankexp_base,
			tankexp_dirt,
			tankexp_snow,
			tankexp_stone,
			water_ripple_moving,
			water_ripple_still,
			water_trail_bubble,
			max
		};

		/**
		 * Return the model path from an effect.
		 *
		 * @param effect The client game module effect.
		 * @return the model path, i.e "/models/fx/grenexp_mud.tik".
		 */
		MOHPC_NET_EXPORTS const char* getEffectName(effects_e effect);
	}
}
}