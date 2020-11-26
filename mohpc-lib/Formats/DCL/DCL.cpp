#include <Shared.h>
#include <MOHPC/Formats/DCL.h>
#include <MOHPC/Managers/FileManager.h>
#include <MOHPC/Managers/AssetManager.h>
#include <MOHPC/Managers/ShaderManager.h>
#include <MOHPC/Math.h>
#include <MOHPC/Misc/Endian.h>
#include <MOHPC/Misc/EndianHelpers.h>
#include <MOHPC/Log.h>

#include <cstring>

#define MOHPC_LOG_NAMESPACE "dcl"

using namespace MOHPC;

namespace MOHPC
{
static constexpr char DCL_SIGNATURE[] = { 'D', 'C', 'L', ' ' };

	enum class dclVersion_e
	{
		Beta = 1,
		Final_100
	};

	typedef struct {
		char ident[4];
		uint32_t version;
		uint32_t checksum;
		uint32_t iNumDecals;
		uint32_t iNumFragments;
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

void DCL::Load()
{
	FilePtr File = GetFileManager()->OpenFile(GetFilename().c_str());
	if (!File) {
		throw AssetError::AssetNotFound(GetFilename());
	}

	dclHeader_t dclHeader;
	File->GetStream()->read((char*)&dclHeader, sizeof(dclHeader_t));

	if (memcmp(dclHeader.ident, DCL_SIGNATURE, sizeof(dclHeader.ident))) {
		throw DCLError::BadHeader((const uint8_t*)dclHeader.ident);
	}

	const dclVersion_e version = dclVersion_e(Endian.LittleInteger(dclHeader.version));
	if (version != dclVersion_e::Beta && version != dclVersion_e::Final_100) {
		throw DCLError::WrongVersion((uint32_t)version);
	}

	char mapTime[33];

	if (version == dclVersion_e::Beta)
	{
		// no map time in beta
		mapTime[0] = '\0';
	}
	else
	{
		File->GetStream()->read(mapTime, sizeof(mapTime) - 1);
		mapTime[32] = '\0';
	}

	char lastShaderName[64] = { 0 };

	numDecals = Endian.LittleInteger(dclHeader.iNumDecals);
	numFragments = Endian.LittleInteger(dclHeader.iNumFragments);

	dclDecals = new DCLMarkDef[numDecals];

	const ShaderManagerPtr shaderManager = GetManager<ShaderManager>();

	for (size_t i = 0; i < numDecals; i++)
	{
		dclSavedMarkDef_t saveMark{0};

		// read each decals one by one
		File->GetStream()->read((char*)&saveMark, sizeof(dclSavedMarkDef_t));

		DCLMarkDef* const pPoly = &dclDecals[i];
		pPoly->shader = shaderManager->GetShader(saveMark.shader);
		pPoly->position = EndianHelpers::LittleVector(Endian, saveMark.vPos);
		pPoly->projection = EndianHelpers::LittleVector(Endian, saveMark.vProjection);
		pPoly->radius = Endian.LittleFloat(saveMark.fRadius);
		pPoly->heightScale = Endian.LittleFloat(saveMark.fHeightScale);
		pPoly->widthScale = Endian.LittleFloat(saveMark.fWidthScale);
		pPoly->rotation = Endian.LittleFloat(saveMark.fRotation);
		saveMark.color[0] = Endian.LittleFloat(pPoly->color[0]);
		saveMark.color[1] = Endian.LittleFloat(pPoly->color[1]);
		saveMark.color[2] = Endian.LittleFloat(pPoly->color[2]);
		saveMark.color[3] = Endian.LittleFloat(pPoly->color[3]);
		pPoly->bDoLighting = saveMark.bDoLighting;
	}

	MOHPC_LOG(Info, "%d decals loaded for '%s'.", numDecals, GetFilename().c_str());

	// FIXME: Should it process numFragments?
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

DCLError::BadHeader::BadHeader(const uint8_t inHeader[4])
	: foundHeader{ inHeader[0], inHeader[1], inHeader[2], inHeader[3] }
{
}

const uint8_t* DCLError::BadHeader::getHeader() const
{
	return foundHeader;
}

const char* DCLError::BadHeader::what() const
{
	return "Bad DCL header";
}

DCLError::WrongVersion::WrongVersion(uint32_t inVersion)
	: version(inVersion)
{
}

uint32_t DCLError::WrongVersion::getVersion() const
{
	return version;
}

const char* DCLError::WrongVersion::what() const
{
	return "Wrong DCL version";
}
