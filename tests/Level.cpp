#include <MOHPC/Assets/Formats/BSP.h>
#include <MOHPC/Assets/Formats/DCL.h>
#include <MOHPC/Assets/Managers/AssetManager.h>
#include <MOHPC/Assets/Managers/ShaderManager.h>
#include <MOHPC/Utility/Collision/Collision.h>
#include <MOHPC/Utility/Collision/CollisionArchive.h>
#include "Common/Common.h"

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
	mfuse::con::Container<uint8_t> data;
	size_t pos;

public:
	ArchiveWriter()
		: pos(0)
	{
	}

	void serialize(void* value, size_t size) override
	{
		if (pos + size >= data.NumObjects()) {
			data.SetNumObjectsUninitialized(data.NumObjects() * 2 + size);
		}

		memcpy(data.Data() + pos, value, size);
		pos += size;
	}

	const mfuse::con::Container<uint8_t>& getData() const
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
	const MOHPC::AssetManagerPtr AM = AssetLoad();

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
		Vector start(1011.12500f, 1136.81250f, 116.125000f);
		Vector end(1011.12500f, 1136.81250f, 98.1250000f);
		cm->BoxTrace(&results, start, end, Vector(-15, -15, 0), Vector(15, 15, 96), 0, ContentFlags::MASK_PLAYERSOLID, true);
	}
	{

		Vector start(-511, 260, 97);
		Vector end(-520, 0, -1000);
		cm->BoxTrace(&results, start, end, Vector(0, 0, 0), Vector(0, 0, 0), 0, ContentFlags::MASK_SHOT, false);
	}
	{

		Vector start(-882, 2690, 82);
		Vector end(infinite, -infinite, -infinite);
		cm->BoxTrace(&results, start, end, Vector(0, 0, 0), Vector(0, 0, 0), 0, ContentFlags::MASK_SHOT, false);
	}
	{

		Vector start(-511, 260, 97);
		Vector end(infinite, -infinite, -infinite);
		cm->BoxTrace(&results, start, end, Vector(0, 0, 0), Vector(0, 0, 0), 0, ContentFlags::MASK_SHOT, false);
	}

	// Patch testing
	{
		Vector start(499.133942f, -427.044525f, -151.875000f);
		Vector end(499.125824f, -426.720612f, -151.875000f);
		Vector mins(-15, -15, 0);
		Vector maxs(15, 15, 96);
		Vector origin(476.f, -400.f, -150.f);

		//cm.BoxTrace(&results, start, end, Vector(-15, -15, 0), Vector(15, 15, 96), 0, ContentFlags::MASK_PLAYERSOLID, true);
		cm->TransformedBoxTrace(&results, start, end, mins, maxs, 37, ContentFlags::MASK_PLAYERSOLID, origin, vec_origin, true);
		assert(results.fraction < 0.01f);
	}

	Vector start(0, 0, 0);
	Vector end(0, 0, -500);
	cm->BoxTrace(&results, start, end, Vector(), Vector(), 0, ContentFlags::MASK_PLAYERSOLID, true);
	assert(results.fraction < 0.3f);

	ArchiveWriter ar;
	CollisionWorldSerializer colSer(*cm);
	colSer.save(ar);

	const mfuse::con::Container<uint8_t>& data = ar.getData();

	ArchiveReader arReader(data.Data(), data.NumObjects());
	colSer.load(arReader);

	trace_t newResults;
	cm->BoxTrace(&newResults, start, end, Vector(), Vector(), 0, ContentFlags::MASK_PLAYERSOLID, true);
	assert(newResults.fraction == results.fraction
		&& newResults.surfaceFlags == results.surfaceFlags
		&& newResults.shaderNum == results.shaderNum
		&& newResults.endpos == results.endpos);
}

void leafTesting(MOHPC::BSPPtr Asset)
{
	uintptr_t leafNum = Asset->PointLeafNum(MOHPC::Vector(0, 0, 0));

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
