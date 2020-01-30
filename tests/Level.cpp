#include <MOHPC/Formats/BSP.h>
#include <MOHPC/Formats/DCL.h>
#include <MOHPC/Managers/AssetManager.h>

void TestLevel(MOHPC::AssetManager& AM)
{
	MOHPC::BSPPtr Level = AM.LoadAsset<MOHPC::BSP>("/maps/lib/mp_anzio_lib.bsp");
	auto Asset = AM.LoadAsset<MOHPC::BSP>("/maps/dm/mohdm2.bsp");
	MOHPC::DCLPtr DCL = AM.LoadAsset<MOHPC::DCL>("/maps/dm/mohdm4.dcl");
}