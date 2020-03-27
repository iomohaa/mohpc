#include <MOHPC/Formats/TIKI.h>
#include <MOHPC/Managers/AssetManager.h>

void TestTiki(MOHPC::AssetManager& AM)
{
	auto Asset = AM.LoadAsset<MOHPC::TIKI>("/models/weapons/m1_garand.tik");
	Asset = AM.LoadAsset<MOHPC::TIKI>("/models/static/bush_full.tik");
	Asset = AM.LoadAsset<MOHPC::TIKI>("/models/human/allied_airborne_soldier.tik");
}
