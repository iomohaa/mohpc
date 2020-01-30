#include <Shared.h>
#include <MOHPC/Formats/DCL.h>
#include <MOHPC/Managers/FileManager.h>
#include <MOHPC/Managers/AssetManager.h>
#include <MOHPC/Managers/ShaderManager.h>
#include <MOHPC/Math.h>

using namespace MOHPC;

namespace MOHPC
{
#define DCL_SIGNATURE 0x204C4344

	enum dclVersion_e
	{
		DCL_VERSION_BETA = 1,
		DCL_VERSION_AA
	};

	typedef struct {
		int32_t ident;
		int32_t version;
		int32_t checksum;
		int32_t iNumDecals;
		int32_t iNumFragments;
	} dclHeader_t;

	typedef struct {
		char shader[64];
		vec3_t vPos;
		vec3_t vProjection;
		float fRadius;
		float fHeightScale;
		float fWidthScale;
		float fRotation;
		vec4_t color;
		int32_t bDoLighting;
	} dclSavedMarkDef_t;

	typedef struct {
		char shader[64];
		int32_t fogIndex;
		int32_t iIndex;
		int32_t iNumVerts;
	} dclSavedMarkPoly_t;
}

CLASS_DEFINITION(DCL);
DCL::DCL()
{
	dclDecals = nullptr;
	numDecals = 0;
}

DCL::~DCL()
{
	if (dclDecals)
	{
		delete[] dclDecals;
	}
}

bool DCL::Load()
{
	FilePtr File = GetFileManager()->OpenFile(GetFilename().c_str());
	if (!File)
	{
		return false;
	}

	dclHeader_t dclHeader;
	File->GetStream()->read((char*)&dclHeader, sizeof(dclHeader_t));

	if (dclHeader.ident != DCL_SIGNATURE)
	{
		return false;
	}

	if (dclHeader.version != DCL_VERSION_BETA && dclHeader.version != DCL_VERSION_AA)
	{
		return false;
	}

	char mapTime[33];

	if (dclHeader.version == DCL_VERSION_BETA)
	{
		mapTime[0] = '\0';
	}
	else
	{
		File->GetStream()->read(mapTime, sizeof(mapTime) - 1);
		mapTime[32] = '\0';
	}

	char lastShaderName[64] = { 0 };

	numDecals = dclHeader.iNumDecals;
	dclDecals = new DCLMarkDef[numDecals];

	for (size_t i = 0; i < numDecals; i++)
	{
		dclSavedMarkDef_t saveMark;

		File->GetStream()->read((char*)&saveMark, sizeof(dclSavedMarkDef_t));

		DCLMarkDef* pPoly = &dclDecals[i];
		pPoly->shader = GetManager<ShaderManager>()->GetShader(saveMark.shader);
		pPoly->position = saveMark.vPos;
		pPoly->projection = saveMark.vProjection;
		pPoly->radius = saveMark.fRadius;
		pPoly->heightScale = saveMark.fHeightScale;
		pPoly->widthScale = saveMark.fWidthScale;
		pPoly->rotation = saveMark.fRotation;
		Vec4Copy(saveMark.color, pPoly->color);
		pPoly->bDoLighting = saveMark.bDoLighting;
	}

	return true;
}

size_t DCL::GetNumDecals() const
{
	return numDecals;
}

const DCLMarkDef* DCL::GetDecal(size_t index) const
{
	if (index < numDecals)
	{
		return &dclDecals[index];
	}
	else
	{
		return nullptr;
	}
}
