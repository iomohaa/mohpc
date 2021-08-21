#include <Shared.h>
#include <MOHPC/Assets/Formats/DCL.h>
#include <MOHPC/Assets/Managers/AssetManager.h>
#include <MOHPC/Assets/Managers/ShaderManager.h>
#include <MOHPC/Files/File.h>
#include <MOHPC/Common/Math.h>
#include <MOHPC/Utility/Misc/Endian.h>
#include <MOHPC/Utility/Misc/EndianCoordHelpers.h>
#include <MOHPC/Common/Log.h>

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

MOHPC_OBJECT_DEFINITION(DCL);
DCL::DCL(const fs::path& fileName, DCLMarkDef* decalList, size_t numDecalsVal, size_t numFragmentsVal)
	: Asset(fileName)
	, dclDecals(decalList)
	, numDecals(numDecalsVal)
	, numFragments(numFragmentsVal)
{
}

DCL::~DCL()
{
	if (dclDecals)
	{
		delete[] dclDecals;
	}
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

MOHPC_OBJECT_DEFINITION(DCLReader);

DCLReader::DCLReader()
{
}

DCLReader::~DCLReader()
{
}

AssetPtr DCLReader::read(const IFilePtr& file)
{
	dclHeader_t dclHeader;
	file->GetStream()->read((char*)&dclHeader, sizeof(dclHeader_t));

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
		file->GetStream()->read(mapTime, sizeof(mapTime) - 1);
		mapTime[32] = '\0';
	}

	char lastShaderName[64] = { 0 };

	const size_t numDecals = Endian.LittleInteger(dclHeader.iNumDecals);
	const size_t numFragments = Endian.LittleInteger(dclHeader.iNumFragments);

	DCLMarkDef* dclDecals = new DCLMarkDef[numDecals];

	const ShaderManagerPtr shaderManager = getManager<ShaderManager>();

	for (size_t i = 0; i < numDecals; i++)
	{
		dclSavedMarkDef_t saveMark{ 0 };

		// read each decals one by one
		file->GetStream()->read((char*)&saveMark, sizeof(dclSavedMarkDef_t));

		DCLMarkDef* const pPoly = &dclDecals[i];
		pPoly->shader = shaderManager->GetShader(saveMark.shader);
		EndianHelpers::LittleVector(Endian, saveMark.vPos, pPoly->position);
		EndianHelpers::LittleVector(Endian, saveMark.vProjection, pPoly->projection);
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

	MOHPC_LOG(Info, "%d decals loaded for '%s'.", numDecals, file->getName().generic_string().c_str());

	return AssetPtr(new DCL(file->getName(), dclDecals, numDecals, numFragments));
}

DCLError::BadHeader::BadHeader(const uint8_t inHeader[4])
	: foundHeader{ inHeader[0], inHeader[1], inHeader[2], inHeader[3] }
{
}

const uint8_t* DCLError::BadHeader::getHeader() const
{
	return foundHeader;
}

const char* DCLError::BadHeader::what() const noexcept
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

const char* DCLError::WrongVersion::what() const noexcept
{
	return "Wrong DCL version";
}
