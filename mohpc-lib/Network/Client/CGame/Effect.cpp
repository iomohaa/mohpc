#include <MOHPC/Network/Client/CGame/Effect.h>

#include <cstdint>
#include <cstddef>

using namespace MOHPC;
using namespace MOHPC::Network;
using namespace MOHPC::Network::CGame;

const char* effectsModel[] =
{
"models/fx/barrel_oil_leak_big.tik",
"models/fx/barrel_oil_leak_medium.tik",
"models/fx/barrel_oil_leak_small.tik",
"models/fx/barrel_oil_leak_splat.tik",
"models/fx/barrel_water_leak_big.tik",
"models/fx/barrel_water_leak_medium.tik",
"models/fx/barrel_water_leak_small.tik",
"models/fx/barrel_water_leak_splat.tik",
"models/fx/bazookaexp_base.tik",
"models/fx/bazookaexp_dirt.tik",
"models/fx/bazookaexp_snow.tik",
"models/fx/bazookaexp_stone.tik",
"models/fx/bh_carpet_hard.tik",
"models/fx/bh_carpet_lite.tik",
"models/fx/bh_dirt_hard.tik",
"models/fx/bh_dirt_lite.tik",
"models/fx/bh_foliage_hard.tik",
"models/fx/bh_foliage_lite.tik",
"models/fx/bh_glass_hard.tik",
"models/fx/bh_glass_lite.tik",
"models/fx/bh_grass_hard.tik",
"models/fx/bh_grass_lite.tik",
"models/fx/bh_human_uniform_hard.tik",
"models/fx/bh_human_uniform_lite.tik",
"models/fx/bh_metal_hard.tik",
"models/fx/bh_metal_lite.tik",
"models/fx/bh_mud_hard.tik",
"models/fx/bh_mud_lite.tik",
"models/fx/bh_paper_hard.tik",
"models/fx/bh_paper_lite.tik",
"models/fx/bh_sand_hard.tik",
"models/fx/bh_sand_lite.tik",
"models/fx/bh_snow_hard.tik",
"models/fx/bh_snow_lite.tik",
"models/fx/bh_stone_hard.tik"
"models/fx/bh_stone_lite.tik",
"models/fx/bh_water_hard.tik",
"models/fx/bh_water_lite.tik",
"models/fx/bh_wood_hard.tik",
"models/fx/bh_wood_lite.tik",
"models/fx/fs_dirt.tik",
"models/fx/fs_grass.tik",
"models/fx/fs_heavy_dust.tik",
"models/fx/fs_light_dust.tik",
"models/fx/fs_mud.tik",
"models/fx/fs_puddle.tik",
"models/fx/fs_sand.tik",
"models/fx/fs_snow.tik",
"models/fx/fx_fence_wood.tik",
"models/fx/grenexp_base.tik",
"models/fx/grenexp_carpet.tik",
"models/fx/grenexp_dirt.tik",
"models/fx/grenexp_foliage.tik",
"models/fx/grenexp_grass.tik",
"models/fx/grenexp_gravel.tik",
"models/fx/grenexp_metal.tik",
"models/fx/grenexp_mud.tik",
"models/fx/grenexp_paper.tik",
"models/fx/grenexp_sand.tik",
"models/fx/grenexp_snow.tik",
"models/fx/grenexp_stone.tik",
"models/fx/grenexp_water.tik",
"models/fx/grenexp_wood.tik",
"models/fx/heavyshellexp_base.tik",
"models/fx/heavyshellexp_dirt.tik",
"models/fx/heavyshellexp_snow.tik",
"models/fx/heavyshellexp_stone.tik",
"models/fx/tankexp_base.tik",
"models/fx/tankexp_dirt.tik",
"models/fx/tankexp_snow.tik",
"models/fx/tankexp_stone.tik",
"models/fx/water_ripple_moving.tik",
"models/fx/water_ripple_still.tik",
"models/fx/water_trail_bubble.tik"
};

const char* MOHPC::Network::CGame::getEffectName(effects_e effect)
{
	return effectsModel[(size_t)effect];
}
