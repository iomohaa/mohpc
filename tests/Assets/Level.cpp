#include <MOHPC/Assets/Formats/BSP.h>
#include <MOHPC/Assets/Formats/DCL.h>
#include <MOHPC/Assets/Managers/AssetManager.h>
#include <MOHPC/Assets/Managers/ShaderManager.h>
#include <MOHPC/Utility/Collision/Collision.h>
#include <MOHPC/Utility/Collision/CollisionArchive.h>
#include "Common/Common.h"

#include <cassert>
#include <map>
#include <vector>

class ArchiveReader : public MOHPC::IArchiveReader
{
private:
	const uint8_t* data;
	size_t dataSize;
	size_t dataPos;

public:
	ArchiveReader(const uint8_t* inData, size_t inDataSize)
		: data(inData)
		, dataSize(inDataSize)
		, dataPos(0)
	{

	}

	void serialize(void* value, size_t size) override
	{
		memcpy(value, data + dataPos, size);
		dataPos += size;
	}
};

class ArchiveWriter : public MOHPC::IArchiveWriter
{
private:
	std::vector<uint8_t> data;
	size_t pos;

public:
	ArchiveWriter()
		: pos(0)
	{
	}

	void serialize(void* value, size_t size) override
	{
		if (pos + size >= data.size()) {
			data.resize(data.size() * 2 + size);
		}

		memcpy(data.data() + pos, value, size);
		pos += size;
	}

	const std::vector<uint8_t>& getData() const
	{
		return data;
	}
};

/*
template<typename T>
Archive& operator<<(Archive& ar, const T& obj)
{
	obj.serialize(ar);
}

template<typename T>
Archive& operator>>(Archive& ar, const T& obj)
{
	obj.serialize(ar);
}
*/


void traceTest(MOHPC::BSPPtr Asset);
void leafTesting(MOHPC::BSPPtr Asset);

int main(int argc, const char* argv[])
{
	InitCommon();
	const MOHPC::AssetManagerPtr AM = AssetLoad(GetGamePathFromCommandLine(argc, argv));

	MOHPC::DCLPtr DCL = AM->LoadAsset<MOHPC::DCL>("/maps/dm/mohdm4.dcl");
	MOHPC::DCLPtr DCLBT = AM->LoadAsset<MOHPC::DCL>("/maps/e1l1.dcl");

	//MOHPC::BSPPtr Asset = AM->LoadAsset<MOHPC::BSP>("/maps/dm/mp_stadt_dm.bsp");
	MOHPC::BSPPtr Asset = AM->LoadAsset<MOHPC::BSP>("/maps/e1l1.bsp");
	if (Asset)
	{
		traceTest(Asset);
		leafTesting(Asset);

		MOHPC::BSPData::TerrainCollide collision;
		Asset->GenerateTerrainCollide(Asset->GetTerrainPatch(0), collision);
	}
}

void traceTest(MOHPC::BSPPtr Asset)
{
	using namespace MOHPC;
	CollisionWorldPtr cm = CollisionWorld::create();
	Asset->FillCollisionWorld(*cm);

	union {
		float infinite;
		uint32_t intFloat;
	};

	intFloat = ~0u;

	trace_t results;
	{
		vec3_t start{ 1011.12500f, 1136.81250f, 116.125000f };
		vec3_t end{ 1011.12500f, 1136.81250f, 98.1250000f };
		cm->BoxTrace(&results, start, end, vec3_t{ -15, -15, 0 }, vec3_t{ 15, 15, 96 }, 0, ContentFlags::MASK_PLAYERSOLID, true);
	}
	{

		vec3_t start{ -511, 260, 97 };
		vec3_t end{ -520, 0, -1000 };
		cm->BoxTrace(&results, start, end, vec3_zero, vec3_zero, 0, ContentFlags::MASK_SHOT, false);
	}
	{

		vec3_t start{-882, 2690, 82};
		vec3_t end{ infinite, -infinite, -infinite };
		cm->BoxTrace(&results, start, end, vec3_zero, vec3_zero, 0, ContentFlags::MASK_SHOT, false);
	}
	{

		vec3_t start{ -511, 260, 97 };
		vec3_t end{ infinite, -infinite, -infinite };
		cm->BoxTrace(&results, start, end, vec3_zero, vec3_zero, 0, ContentFlags::MASK_SHOT, false);
	}

	// Patch testing
	{
		vec3_t start{ 499.133942f, -427.044525f, -151.875000f };
		vec3_t end{ 499.125824f, -426.720612f, -151.875000f };
		vec3_t mins{ -15, -15, 0 };
		vec3_t maxs{ 15, 15, 96 };
		vec3_t origin{ 476.f, -400.f, -150.f };

		//cm.BoxTrace(&results, start, end, vec3_t(-15, -15, 0), vec3_t(15, 15, 96), 0, ContentFlags::MASK_PLAYERSOLID, true);
		cm->TransformedBoxTrace(&results, start, end, mins, maxs, 37, ContentFlags::MASK_PLAYERSOLID, origin, vec3_origin, true);
		assert(results.fraction < 0.01f);
	}

	vec3_t start{ 0, 0, 0 };
	vec3_t end{ 0, 0, -500 };
	cm->BoxTrace(&results, start, end, vec3_zero, vec3_zero, 0, ContentFlags::MASK_PLAYERSOLID, true);
	assert(results.fraction < 0.3f);

	ArchiveWriter ar;
	CollisionWorldSerializer colSer(*cm);
	colSer.save(ar);

	const std::vector<uint8_t>& data = ar.getData();

	ArchiveReader arReader(data.data(), data.size());
	colSer.load(arReader);

	trace_t newResults;
	cm->BoxTrace(&newResults, start, end, vec3_zero, vec3_zero, 0, ContentFlags::MASK_PLAYERSOLID, true);
	assert(newResults.fraction == results.fraction);
	assert(newResults.surfaceFlags == results.surfaceFlags);
	assert(newResults.shaderNum == results.shaderNum);
	assert(newResults.endpos[0] == results.endpos[0]);
	assert(newResults.endpos[1] == results.endpos[1]);
	assert(newResults.endpos[2] == results.endpos[2]);
}

void leafTesting(MOHPC::BSPPtr Asset)
{
	using namespace MOHPC;
	uintptr_t leafNum = Asset->PointLeafNum(vec3_zero);

	std::map<uintptr_t, uintptr_t> brushRefs;
	std::vector<std::vector<const MOHPC::BSPData::Brush*>> brushArrays;

	size_t numLeafs = Asset->GetNumLeafs();
	brushArrays.resize(numLeafs);

	for (size_t i = 0; i < numLeafs; ++i)
	{
		const MOHPC::BSPData::Leaf* leaf = Asset->GetLeaf(i);

		for (size_t j = 0; j < leaf->numLeafBrushes; ++j)
		{
			uintptr_t brushNum = Asset->GetLeafBrush(leaf->firstLeafBrush + j);

			const size_t brushRef = brushRefs[brushNum]++;
			if (!brushRef)
			{
				const MOHPC::BSPData::Brush* brush = Asset->GetBrush(brushNum);
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
