#include <MOHPC/Formats/BSP.h>
#include <MOHPC/Formats/DCL.h>
#include <MOHPC/Managers/AssetManager.h>
#include "UnitTest.h"

#include <map>
#include <vector>

class CLevelTest : public IUnitTest, public TAutoInst<CLevelTest>
{
public:
	virtual const char* name() override
	{
		return "BSP";
	}

	virtual void run(MOHPC::AssetManager& AM) override
	{
		//MOHPC::BSPPtr Level = AM.LoadAsset<MOHPC::BSP>("/maps/lib/mp_anzio_lib.bsp");
		MOHPC::BSPPtr Asset = AM.LoadAsset<MOHPC::BSP>("/maps/dm/mohdm2.bsp");
		leafTesting(Asset);
		MOHPC::DCLPtr DCL = AM.LoadAsset<MOHPC::DCL>("/maps/dm/mohdm4.dcl");

		MOHPC::BSP::TerrainCollide collision;
		Asset->GenerateTerrainCollide(Asset->GetTerrainPatch(0), collision);
	}

	void leafTesting(MOHPC::BSPPtr Asset)
	{
		uintptr_t leafNum = Asset->PointLeafNum(MOHPC::Vector(0, 0, 0));

		std::map<uintptr_t, uintptr_t> brushRefs;
		std::vector<std::vector<const MOHPC::BSP::Brush*>> brushArrays;

		size_t numLeafs = Asset->GetNumLeafs();
		brushArrays.resize(numLeafs);

		for (size_t i = 0; i < numLeafs; ++i)
		{
			const MOHPC::BSP::Leaf* leaf = Asset->GetLeaf(i);

			for (size_t j = 0; j < leaf->numLeafBrushes; ++j)
			{
				uintptr_t brushNum = Asset->GetLeafBrush(leaf->firstLeafBrush + j);

				const size_t brushRef = brushRefs[brushNum]++;
				if (!brushRef) {
					const MOHPC::BSP::Brush* brush = Asset->GetBrush(brushNum);
					brushArrays[i].push_back(brush);
				}
			}
		}

		for (auto it = brushArrays.begin(); it != brushArrays.end(); )
		{
			if (!it->size()) {
				it = brushArrays.erase(it);
			}
			else {
				++it;
			}
		}
	}
};
