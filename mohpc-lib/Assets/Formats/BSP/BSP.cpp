#include <Shared.h>
#include <MOHPC/Assets/Formats/BSP.h>
#include <MOHPC/Assets/Formats/BSP_Collision.h>
#include <MOHPC/Assets/Script.h>
#include <MOHPC/Common/Vector.h>
#include <MOHPC/Utility/LevelEntity.h>
#include <MOHPC/Common/Log.h>
#include <MOHPC/Files/Managers/FileManager.h>
#include <MOHPC/Assets/Managers/AssetManager.h>
#include <MOHPC/Assets/Managers/ShaderManager.h>
#include <MOHPC/Utility/Collision/Collision.h>
#include <MOHPC/Utility/Misc/Endian.h>
#include <MOHPC/Utility/Misc/EndianCoordHelpers.h>

#include "BSP_Curve.h"

#include "../../../Common/VectorPrivate.h"
#include <Eigen/Geometry>

#include <chrono>
#include <algorithm>
#include <functional>

/*
#ifdef DEBUG
#define BEGIN_LOAD_PROFILING(name) \
	{ \
		auto start = std::chrono::system_clock().now(); \
		const str profileName = name

#define END_LOAD_PROFILING() \
		auto end = std::chrono::system_clock().now();
		printf("%lf time (" ##name ")\n", std::chrono::duration<double>(end - start).count());
	}

#endif
*/

#define MOHPC_LOG_NAMESPACE "bsp_asset"

static constexpr unsigned int MIN_MAP_SUBDIVISIONS = 16;

static void ProfilableCode(const char *profileName, std::function<void()> Lambda)
{
#ifndef NDEBUG
	auto start = std::chrono::system_clock().now();
	{
		Lambda();
	}
	auto end = std::chrono::system_clock().now();

	MOHPC_LOG(Debug, "%lf time (%s)", std::chrono::duration<double>(end - start).count(), profileName);
#else
	Lambda();
#endif
}

namespace MOHPC
{
	// little-endian "2015"
static constexpr unsigned char BSP_IDENT[]				= "2015";
	// little-endian "EALA"
static constexpr unsigned char BSP_EXPANSIONS_IDENT[]	= "EALA"; 

static constexpr unsigned int BSP_BETA_VERSION		= 18;	// Beta Allied Assault
static constexpr unsigned int BSP_BASE_VERSION		= 19;	// vanilla Allied Assault
static constexpr unsigned int BSP_VERSION			= 19;	// current Allied Assault
static constexpr unsigned int BSP_MAX_VERSION		= 21;	// MOH:BT

static constexpr unsigned int LUMP_SHADERS				= 0;
static constexpr unsigned int LUMP_PLANES				= 1;
static constexpr unsigned int LUMP_LIGHTMAPS			= 2;
static constexpr unsigned int LUMP_SURFACES				= 3;
static constexpr unsigned int LUMP_DRAWVERTS			= 4;
static constexpr unsigned int LUMP_DRAWINDEXES			= 5;
static constexpr unsigned int LUMP_LEAFBRUSHES			= 6;
static constexpr unsigned int LUMP_LEAFSURFACES			= 7;
static constexpr unsigned int LUMP_LEAFS				= 8;
static constexpr unsigned int LUMP_NODES				= 9;
static constexpr unsigned int LUMP_SIDEEQUATIONS		= 10;
static constexpr unsigned int LUMP_BRUSHSIDES			= 11;
static constexpr unsigned int LUMP_BRUSHES				= 12;
static constexpr unsigned int LUMP_MODELS				= 13;
static constexpr unsigned int LUMP_ENTITIES				= 14;
static constexpr unsigned int LUMP_VISIBILITY			= 15;
static constexpr unsigned int LUMP_LIGHTGRIDPALETTE		= 16;
static constexpr unsigned int LUMP_LIGHTGRIDOFFSETS		= 17;
static constexpr unsigned int LUMP_LIGHTGRIDDATA		= 18;
static constexpr unsigned int LUMP_SPHERELIGHTS			= 19;
static constexpr unsigned int LUMP_SPHERELIGHTVIS		= 20;
static constexpr unsigned int LUMP_LIGHTDEFS			= 21;
static constexpr unsigned int LUMP_TERRAIN				= 22;
static constexpr unsigned int LUMP_TERRAININDEXES		= 23;
static constexpr unsigned int LUMP_STATICMODELDATA		= 24;
static constexpr unsigned int LUMP_STATICMODELDEF		= 25;
static constexpr unsigned int LUMP_STATICMODELINDEXES	= 26;
static constexpr unsigned int LUMP_DUMMY10				= 27;

static constexpr unsigned int HEADER_LUMPS = 28;

static constexpr unsigned int VIS_HEADER = 8;
static constexpr unsigned int LIGHTMAP_SIZE = 128;
static constexpr float TERRAIN_LIGHTMAP_LENGTH = (16.f / LIGHTMAP_SIZE);

namespace BSPFile
{
	struct GameLump
	{
		void* buffer;
		size_t length;

	public:
		GameLump();
		~GameLump();
	};

	struct flump_t
	{
		uint32_t fileOffset;
		uint32_t fileLength;
	};

	struct fheader_t
	{
		uint8_t ident[4];
		uint32_t version;
		uint32_t checksum;

		flump_t lumps[HEADER_LUMPS];
	};

	struct fshader_t
	{
		char shader[64];
		uint32_t surfaceFlags;
		uint32_t contentFlags;
		uint32_t subdivisions;
		char fenceMaskImage[64];
	};

	struct fsurface_t
	{
		uint32_t shaderNum;
		uint32_t fogNum;
		uint32_t surfaceType;
		uint32_t firstVert;
		uint32_t numVerts;
		uint32_t firstIndex;
		uint32_t numIndexes;
		uint32_t lightmapNum;
		uint32_t lightmapX, lightmapY;
		uint32_t lightmapWidth, lightmapHeight;
		float lightmapOrigin[3];
		float lightmapVecs[3][3];
		uint32_t patchWidth;
		uint32_t patchHeight;
		float subdivisions;
	};

	struct fvertice_t
	{
		float xyz[3];
		float st[2];
		float lightmap[2];
		float normal[3];
		uint8_t color[4];
	};

	struct fplane_t
	{
		float normal[3];
		float dist;
	};

	struct fsideequation_t
	{
		float seq[4];
		float teq[4];
	};

	struct fbrushSide_t
	{
		uint32_t planeNum;
		uint32_t shaderNum;
		uint32_t equationNum;
	};

	struct fbrush_t
	{
		uint32_t firstSide;
		uint32_t numSides;
		uint32_t shaderNum;
	};

	struct fbmodel_t
	{
		float mins[3];
		float maxs[3];
		uint32_t firstSurface;
		uint32_t numSurfaces;
		uint32_t firstBrush;
		uint32_t numBrushes;
	};

	struct fsphereLight_t
	{
		float origin[3];
		float color[3];
		float intensity;
		int leaf;
		uint32_t needs_trace;
		uint32_t spot_light;
		float spot_dir[3];
		float spot_radiusbydistance;
	};

	struct fstaticModel_t
	{
		char model[128];
		float origin[3];
		float angles[3];
		float scale;
		uint32_t firstVertexData;
		uint32_t numVertexData;
	};

	struct fterrainPatch_t
	{
		struct VarNode {
			uint16_t flags;
		};

		uint8_t flags;
		uint8_t lmapScale;
		uint8_t s;
		uint8_t t;
		float texCoord[2][2][2];
		int8_t x;
		int8_t y;
		int16_t iBaseHeight;
		uint16_t iShader;
		uint16_t iLightMap;
		int16_t iNorth;
		int16_t iEast;
		int16_t iSouth;
		int16_t iWest;
		VarNode varTree[2][63];
		uint8_t heightmap[9 * 9];
	};

	struct fnode_t {
		uint32_t planeNum;
		uint32_t children[2];
		uint32_t mins[3];
		uint32_t maxs[3];
	};

	struct fleaf_t {
		uint32_t cluster;
		uint32_t area;
		uint32_t mins[3];
		uint32_t maxs[3];
		uint32_t firstLeafSurface;
		uint32_t numLeafSurfaces;
		uint32_t firstLeafBrush;
		uint32_t numLeafBrushes;
		uint32_t firstTerraPatch;
		uint32_t numTerraPatches;
		uint32_t firstStaticModel;
		uint32_t numStaticModels;
	};

	// old leaf version
	struct fleaf_ver17_t {
		uint32_t cluster;
		uint32_t area;
		uint32_t mins[3];
		uint32_t maxs[3];
		uint32_t firstLeafSurface;
		uint32_t numLeafSurfaces;
		uint32_t firstLeafBrush;
		uint32_t numLeafBrushes;
		uint32_t firstTerraPatch;
		uint32_t numTerraPatches;
	};
}

	struct Patch
	{
		Patch* parent;
		const BSPData::Surface* surface;
		vec3_t bounds[2];
		BSPData::SurfacesGroup* surfaceGroup;

		Patch(const BSPData::Surface* inSurface)
			: parent(nullptr)
			, surface(inSurface)
			, surfaceGroup(nullptr)
		{
			const size_t numVerts = surface->GetNumVertices();
			for (size_t i = 0; i < numVerts; ++i)
			{
				AddPointToBounds(surface->GetVertice(i)->xyz, bounds[0], bounds[1]);
			}
		}

		Patch* GetRoot() const
		{
			Patch* p = const_cast<Patch*>(this);
			while (p->parent)
			{
				p = p->parent;
			}

			return p;
		}

		bool HasParent(const Patch* other) const
		{
			Patch* p = parent;
			for(Patch* p = parent; p; p = p->parent)
			{
				if (p == other)
				{
					return true;
				}
			}

			return false;
		}

		bool IsTouching(const Patch* other) const
		{
			if (other->bounds[0][0] > bounds[1][0])
			{
				return false;
			}
			if (other->bounds[0][1] > bounds[1][1])
			{
				return false;
			}
			if (other->bounds[0][2] > bounds[1][2])
			{
				return false;
			}
			if (other->bounds[1][0] < bounds[0][0])
			{
				return false;
			}
			if (other->bounds[1][1] < bounds[0][1])
			{
				return false;
			}
			if (other->bounds[1][2] < bounds[0][2])
			{
				return false;
			}

			return true;
		}
	};
};

using namespace MOHPC;
using namespace BSPData;

BSPFile::GameLump::GameLump()
	: buffer(nullptr)
	, length(0)
{
}

BSPFile::GameLump::~GameLump()
{
	if (buffer) {
		delete[] (uint8_t*)buffer;
	}
}

const char* BSPData::Brush::GetName() const
{
	return name.c_str();
}

const BSPData::Shader* BSPData::Brush::GetShader() const
{
	return shader;
}

int32_t BSPData::Brush::GetContents() const
{
	return contents;
}

const_vec3p_t BSPData::Brush::GetMins() const
{
	return bounds[0];
}

const_vec3p_t BSPData::Brush::GetMaxs() const
{
	return bounds[1];
}

size_t BSPData::Brush::GetNumSides() const
{
	return numsides;
}

const BSPData::BrushSide* BSPData::Brush::GetSide(size_t Index) const
{
	return Index < numsides ? &sides[Index] : nullptr;
}

BSPData::Brush* BSPData::Brush::GetParent() const
{
	return parent;
}

BSPData::PatchCollide::~PatchCollide()
{
	if (planes)
	{
		delete[] planes;
	}

	if (facets)
	{
		delete[] facets;
	}
}

size_t BSPData::Lightmap::GetNumPixels() const
{
	return sizeof(color) / sizeof(color[0]);
}

size_t BSPData::Lightmap::GetWidth() const
{
	return lightmapSize;
}

size_t BSPData::Lightmap::GetHeight() const
{
	return lightmapSize;
}

void BSPData::Lightmap::GetColor(size_t pixelNum, uint8_t(&out)[3]) const
{
	out[0] = color[pixelNum][0];
	out[1] = color[pixelNum][1];
	out[2] = color[pixelNum][2];
}

BSPData::Surface::Surface()
{
	shader = nullptr;
	bIsPatch = false;
	pc = nullptr;
}

BSPData::Surface::~Surface()
{
	if (pc)
	{
		delete pc;
	}
}

const BSPData::Shader* BSPData::Surface::GetShader() const
{
	return shader;
}

size_t BSPData::Surface::GetNumVertices() const
{
	return vertices.size();
}

const BSPData::Vertice *BSPData::Surface::GetVertice(size_t index) const
{
	return &vertices[index];
}


size_t BSPData::Surface::GetNumIndexes() const
{
	return indexes.size();
}

uint32_t BSPData::Surface::GetIndice(size_t index) const
{
	return indexes[index];
}

uint32_t Surface::getWidth() const
{
	return width;
}

uint32_t Surface::getHeight() const
{
	return height;
}

int32_t BSPData::Surface::GetLightmapNum() const
{
	return lightmapNum;
}

int32_t BSPData::Surface::GetLightmapX() const
{
	return lightmapX;
}

int32_t BSPData::Surface::GetLightmapY() const
{
	return lightmapY;
}

int32_t BSPData::Surface::GetLightmapWidth() const
{
	return lightmapWidth;
}

int32_t BSPData::Surface::GetLightmapHeight() const
{
	return lightmapHeight;
}

const_vec3p_t BSPData::Surface::GetLightmapOrigin() const
{
	return lightmapOrigin;
}

const_vec3p_t BSPData::Surface::GetLightmapVec(int32_t num) const
{
	return lightmapVecs[num];
}

const BSPData::PatchCollide* BSPData::Surface::GetPatchCollide() const
{
	return pc;
}

bool BSPData::Surface::IsPatch() const
{
	return bIsPatch;
}

void BSPData::Surface::CalculateCentroid()
{
	Vector3 avgVert = castVector(vec3_zero);
	const size_t numVerts = vertices.size();
	const Vertice *verts = vertices.data();
	for (size_t v = 0; v < numVerts; v++)
	{
		const Vertice* pVert = &verts[v];
		avgVert += castVector(pVert->xyz);
	}

	castVector(centroid) = avgVert / (float)numVerts;
}

void BSPData::Brush::GetOrigin(vec3r_t out) const
{
	Vector3 vec = (castVector(bounds[0]) + castVector(bounds[1])) * 0.5f;
	castVector(out) = vec;
}

const str& BSPData::SurfacesGroup::GetGroupName() const
{
	return name;
}

size_t BSPData::SurfacesGroup::GetNumSurfaces() const
{
	return surfaces.size();
}

const BSPData::Surface *BSPData::SurfacesGroup::GetSurface(size_t index) const
{
	return surfaces.at(index);
}

const BSPData::Surface* const *BSPData::SurfacesGroup::GetSurfaces() const
{
	return surfaces.data();
}

size_t BSPData::SurfacesGroup::GetNumBrushes() const
{
	return brushes.size();
}

const BSPData::Brush *BSPData::SurfacesGroup::GetBrush(size_t index) const
{
	return brushes.at(index);
}

const BSPData::Brush* const *BSPData::SurfacesGroup::GetBrushes() const
{
	return brushes.data();
}

const_vec3p_t BSPData::SurfacesGroup::GetMinBound() const
{
	return bounds[0];
}

const_vec3p_t BSPData::SurfacesGroup::GetMaxBound() const
{
	return bounds[1];
}

const_vec3p_t BSPData::SurfacesGroup::GetOrigin() const
{
	return origin;
}

static bool BrushIsTouching(BSPData::Brush* b1, BSPData::Brush* b2)
{
	if (b1->bounds[0][0] > b2->bounds[1][0])
	{
		return false;
	}
	if (b1->bounds[0][1] > b2->bounds[1][1])
	{
		return false;
	}
	if (b1->bounds[0][2] > b2->bounds[1][2])
	{
		return false;
	}
	if (b1->bounds[1][0] < b2->bounds[0][0])
	{
		return false;
	}
	if (b1->bounds[1][1] < b2->bounds[0][1])
	{
		return false;
	}
	if (b1->bounds[1][2] < b2->bounds[0][2])
	{
		return false;
	}

	return true;
}

MOHPC_OBJECT_DEFINITION(BSP);

BSP::BSP(const fs::path& path)
	: Asset2(path)
{
	numClusters = 0;
	numAreas = 0;
}

BSP::~BSP()
{
	for (size_t i = 0; i < entities.size(); i++)
	{
		delete entities[i];
	}

	for (size_t i = 0; i < surfacesGroups.size(); i++)
	{
		delete surfacesGroups[i];
	}
}

/*
void BSP::PreAllocateLevelData(const File_Header *Header)
{
	size_t memsize;
	memsize = Header->lumps[LUMP_SHADERS].FileLength / sizeof(BSP::File_Shader) * sizeof BSPData::Shader;
	memsize += Header->lumps[LUMP_PLANES].FileLength / sizeof(BSP::File_Plane) * sizeof BSPData::Plane;
	memsize += Header->lumps[LUMP_SURFACES].FileLength / sizeof(BSP::File_Surface) * sizeof BSPData::Surface;
	memsize += Header->lumps[LUMP_DRAWVERTS].FileLength / sizeof(BSP::File_Vertice) * sizeof BSPData::Vertice;
	memsize += Header->lumps[LUMP_DRAWINDEXES].FileLength;
	memsize += Header->lumps[LUMP_SIDEEQUATIONS].FileLength / sizeof(BSP::File_SideEquation) * sizeof BSPData::SideEquation;
	memsize += Header->lumps[LUMP_BRUSHSIDES].FileLength / sizeof(BSP::File_BrushSide) * sizeof BSPData::BrushSide;
	memsize += Header->lumps[LUMP_BRUSHES].FileLength / sizeof(BSP::File_Brush) * sizeof BSPData::Brush;
	memsize += Header->lumps[LUMP_MODELS].FileLength / sizeof(BSP::File_BrushModel) * sizeof BSPData::Model;
	memsize += Header->lumps[LUMP_ENTITIES].FileLength;
	memsize += Header->lumps[LUMP_SPHERELIGHTS].FileLength / sizeof(BSP::File_SphereLight) * sizeof BSPData::SphereLight;
	memsize += Header->lumps[LUMP_TERRAIN].FileLength / sizeof(BSP::File_TerrainPatch) * sizeof BSPData::TerrainPatch;
	memsize += Header->lumps[LUMP_STATICMODELDEF].FileLength / sizeof(BSP::File_StaticModel) * sizeof BSPData::StaticModel;
	memdata = new uint8_t[memsize];
}
*/

BSPData::Plane::PlaneType MOHPC::PlaneTypeForNormal(const_vec3r_t normal)
{
	if (normal[0] == 1.0)
	{
		return Plane::PLANE_X;
	}

	if (normal[1] == 1.0)
	{
		return Plane::PLANE_Y;
	}

	if (normal[2] == 1.0)
	{
		return Plane::PLANE_Z;
	}

	if (normal[0] == 0.0 && normal[1] == 0.0 && normal[2] == 0.0)
	{
		return Plane::PLANE_NON_PLANAR;
	}

	return Plane::PLANE_NON_AXIAL;
}

size_t BSP::GetNumShaders() const
{
	return shaders.size();
}

const BSPData::Shader *BSP::GetShader(size_t shaderNum) const
{
	return &shaders.at(shaderNum);
}

size_t BSP::GetNumLightmaps() const
{
	return lightmaps.size();
}

const BSPData::Lightmap *BSP::GetLightmap(size_t lightmapNum) const
{
	return &lightmaps.at(lightmapNum);
}

size_t BSP::GetNumSurfaces() const
{
	return surfaces.size();
}

const BSPData::Surface *BSP::GetSurface(size_t surfaceNum)
{
	return &surfaces.at(surfaceNum);
}

size_t BSP::GetNumPlanes() const
{
	return planes.size();
}

const BSPData::Plane *BSP::GetPlane(size_t planeNum)
{
	return &planes.at(planeNum);
}

size_t BSP::GetNumSideEquations() const
{
	return sideEquations.size();
}

const BSPData::SideEquation *BSP::GetSideEquation(size_t equationNum)
{
	return &sideEquations.at(equationNum);
}

size_t BSP::GetNumBrushSides() const
{
	return brushSides.size();
}

const BSPData::BrushSide *BSP::GetBrushSide(size_t brushSideNum)
{
	return &brushSides.at(brushSideNum);
}

size_t BSP::GetNumBrushes() const
{
	return brushes.size();
}

const BSPData::Brush *BSP::GetBrush(size_t brushNum) const
{
	return &brushes.at(brushNum);
}

const BSPData::Leaf* BSP::GetLeaf(size_t leafNum) const
{
	return &leafs.at(leafNum);
}

size_t BSP::GetNumLeafs() const
{
	return leafs.size();
}

const BSPData::Node* BSP::GetNode(size_t nodeNum) const
{
	return &nodes.at(nodeNum);
}

size_t BSP::GetNumNodes() const
{
	return nodes.size();
}

uintptr_t BSP::GetLeafBrush(size_t leafBrushNum) const
{
	return leafBrushes.at(leafBrushNum);
}

size_t BSP::GetNumLeafBrushes() const
{
	return leafBrushes.size();
}

uintptr_t BSP::GetLeafSurface(size_t leafSurfNum) const
{
	return leafSurfaces.at(leafSurfNum);
}

size_t BSP::GetNumLeafSurfaces() const
{
	return leafSurfaces.size();
}

size_t BSP::GetNumSubmodels() const
{
	return brushModels.size();
}

const BSPData::TerrainPatch* BSP::GetLeafTerrain(size_t leafTerrainNum) const
{
	return leafTerrains[leafTerrainNum];
}

size_t BSP::GetNumLeafTerrains() const
{
	return leafTerrains.size();
}

const BSPData::Model *BSP::GetSubmodel(size_t submodelNum) const
{
	return &brushModels.at(submodelNum);
}

const BSPData::Model *BSP::GetSubmodel(const str& submodelName) const
{
	if (submodelName.length() > 1 && *submodelName.c_str() == '*')
	{
		const char *strPtr = submodelName.c_str() + 1;
		char *endPtr = nullptr;
		long submodelNum = strtol(strPtr, &endPtr, 10);

		if (endPtr != strPtr)
		{
			return GetSubmodel(submodelNum);
		}
	}

	return nullptr;
}

size_t BSP::GetNumLights() const
{
	return lights.size();
}

const BSPData::SphereLight *BSP::GetLight(size_t lightNum) const
{
	return &lights.at(lightNum);
}

size_t BSP::GetNumStaticModels() const
{
	return staticModels.size();
}

const BSPData::StaticModel *BSP::GetStaticModel(size_t staticModelNum) const
{
	return &staticModels.at(staticModelNum);
}

size_t BSP::GetNumTerrainPatches() const
{
	return terrainPatches.size();
}

const BSPData::TerrainPatch *BSP::GetTerrainPatch(size_t terrainPatchNum) const
{
	return &terrainPatches.at(terrainPatchNum);
}

size_t BSP::GetNumTerrainSurfaces() const
{
	return terrainSurfaces.size();
}

const BSPData::Surface *BSP::GetTerrainSurface(size_t terrainSurfaceNum) const
{
	return &terrainSurfaces.at(terrainSurfaceNum);
}

size_t BSP::GetNumEntities() const
{
	return entities.size();
}

const LevelEntity* BSP::GetEntity(size_t entityNum) const
{
	return entities.at(entityNum);
}

const LevelEntity* BSP::GetEntity(const str& targetName) const
{
	const std::vector<LevelEntity *>* ents = GetEntities(targetName);
	if (ents && ents->size())
	{
		return ents->at(0);
	}
	else
	{
		return nullptr;
	}
}

const std::vector<LevelEntity *>* BSP::GetEntities(const str& targetName) const
{
	/*
	auto it = targetList.find(targetName);
	if (it != targetList.end())
	{
		return &it->second;
	}
	else
	{
		return nullptr;
	}
	*/
	const auto it = targetList.find(targetName);
	if (it != targetList.end()) {
		return &it->second;
	}

	return nullptr;
}

size_t BSP::GetNumSurfacesGroup() const
{
	return surfacesGroups.size();
}

const BSPData::SurfacesGroup *BSP::GetSurfacesGroup(size_t surfsGroupNum) const
{
	return surfacesGroups.at(surfsGroupNum);
}

void BSPReader::LoadShaders(const BSPFile::GameLump* GameLump, std::vector<BSPData::Shader>& shaders)
{
	if (GameLump->length % sizeof(BSPFile::fshader_t)) {
		throw BSPError::FunnyLumpSize("shaders");
	}

	const ShaderManagerPtr shaderManager = GetAssetManager()->getManager<ShaderManager>();

	const size_t count = GameLump->length / sizeof(BSPFile::fshader_t);
	if (count)
	{
		shaders.resize(count);

		const BSPFile::fshader_t* in = (BSPFile::fshader_t *)GameLump->buffer;
		BSPData::Shader* out = shaders.data();

		for (size_t i = 0; i < count; i++, in++, out++)
		{
			out->surfaceFlags = Endian.LittleLong(in->surfaceFlags);
			out->contentFlags = Endian.LittleLong(in->contentFlags);
			out->shaderName = in->shader;
			out->subdivisions = Endian.LittleLong(in->subdivisions);
			out->shader = shaderManager->GetShader(in->shader);
		}
	}
}

void BSPReader::LoadLightmaps(const BSPFile::GameLump* GameLump, std::vector<BSPData::Lightmap>& lightmaps)
{
	if (!GameLump->length) {
		throw BSPError::FunnyLumpSize("lightmaps");
	}

	const size_t numLightmaps = GameLump->length / lightmapMemSize;
	lightmaps.resize(numLightmaps);

	const uint8_t* buf = (uint8_t*)GameLump->buffer;
	for (size_t i = 0; i < numLightmaps; i++)
	{
		Lightmap& lightmap = lightmaps[i];

		const uint8_t* buf_p = buf + i * lightmapMemSize;

		const size_t numPixels = lightmapSize * lightmapSize;
		for (size_t j = 0; j < numPixels; j++)
		{
			lightmap.color[j][0] = buf_p[j * 3 + 0];
			lightmap.color[j][1] = buf_p[j * 3 + 1];
			lightmap.color[j][2] = buf_p[j * 3 + 2];
		}
	}
}

void BSPReader::ParseMesh(const BSPFile::fsurface_t* InSurface, const BSPFile::fvertice_t* InVertices, const std::vector<BSPData::Shader>& shaders, Surface* out)
{
	out->shader = &shaders[Endian.LittleLong(InSurface->shaderNum)];

	int32_t Width = Endian.LittleLong(InSurface->patchWidth);
	int32_t Height = Endian.LittleLong(InSurface->patchHeight);

	if (Width < 0 || Width > MAX_PATCH_SIZE || Height < 0 || Height > MAX_PATCH_SIZE) {
		throw BSPError::BadMeshSize(Width, Height);
	}

	int32_t NumPoints = Width * Height;
	Vertice Points[MAX_PATCH_SIZE * MAX_PATCH_SIZE];

	InVertices += Endian.LittleLong(InSurface->firstVert);
	for (int32_t i = 0; i < NumPoints; i++)
	{
		for (int32_t j = 0; j < 3; j++)
		{
			Points[i].xyz[j] = Endian.LittleFloat(InVertices[i].xyz[j]);
			Points[i].normal[j] = Endian.LittleFloat(InVertices[i].normal[j]);
		}

		AddPointToBounds(Points[i].xyz, out->cullInfo.bounds[0], out->cullInfo.bounds[1]);

		for (int32_t j = 0; j < 2; j++)
		{
			Points[i].st[j] = Endian.LittleFloat(InVertices[i].st[j]);
			Points[i].lightmap[j] = Endian.LittleFloat(InVertices[i].lightmap[j]);
		}

		Points[i].color[0] = InVertices[i].color[0];
		Points[i].color[1] = InVertices[i].color[1];
		Points[i].color[2] = InVertices[i].color[2];
		Points[i].color[3] = InVertices[i].color[3];

		//ColorShiftLightingFloats(color, Points[i].color, 1.0f / 255.0f);
	}

	SubdividePatchToGrid(Width, Height, Points, out);
	out->bIsPatch = true;

	uint32_t subdivisions = out->shader->subdivisions;
	if (subdivisions < MIN_MAP_SUBDIVISIONS) {
		subdivisions = MIN_MAP_SUBDIVISIONS;
	}

	// FIXME
	out->pc = GeneratePatchCollide(Width, Height, Points, (float)subdivisions);
}

void BSPReader::ParseFace(const BSPFile::fsurface_t* InSurface, const BSPFile::fvertice_t* InVertices, const int32_t* InIndices, const std::vector<BSPData::Shader>& shaders, Surface* out)
{
	uint32_t i, j;

	const size_t numVerts = Endian.LittleLong(InSurface->numVerts);
	const size_t numIndexes = Endian.LittleLong(InSurface->numIndexes);

	out->shader = &shaders[Endian.LittleLong(InSurface->shaderNum)];

	out->indexes.resize(numIndexes);
	out->vertices.resize(numVerts);

	// copy vertexes
	out->cullInfo.type = out->cullInfo.CULLINFO_PLANE | out->cullInfo.CULLINFO_BOX;
	ClearBounds(out->cullInfo.bounds[0], out->cullInfo.bounds[1]);
	InVertices += Endian.LittleLong(InSurface->firstVert);
	Vertice *OutVertices = out->vertices.data();
	for (i = 0; i < numVerts; i++)
	{
		for (j = 0; j < 3; j++)
		{
			OutVertices[i].xyz[j] = Endian.LittleFloat(InVertices[i].xyz[j]);
			OutVertices[i].normal[j] = Endian.LittleFloat(InVertices[i].normal[j]);
		}

		AddPointToBounds(OutVertices[i].xyz, out->cullInfo.bounds[0], out->cullInfo.bounds[1]);

		for (j = 0; j < 2; j++)
		{
			OutVertices[i].st[j] = Endian.LittleFloat(InVertices[i].st[j]);
			OutVertices[i].lightmap[j] = Endian.LittleFloat(InVertices[i].lightmap[j]);
		}

		OutVertices[i].color[0] = InVertices[i].color[0];
		OutVertices[i].color[1] = InVertices[i].color[1];
		OutVertices[i].color[2] = InVertices[i].color[2];
		OutVertices[i].color[3] = InVertices[i].color[3];
		//color[0] = InVertices[i].color[0];
		//color[1] = InVertices[i].color[1];
		//color[2] = InVertices[i].color[2];
		//color[3] = InVertices[i].color[3] / 255.0f;

		//ColorShiftLightingFloats(color, OutVertices[i].color, 1.0f / 255.0f);
	}

	// copy triangles
	size_t badTriangles = 0;
	InIndices += Endian.LittleLong(InSurface->firstIndex);
	uint32_t* tri;
	for (i = 0, tri = out->indexes.data(); i < numIndexes; i += 3, tri += 3)
	{
		for (j = 0; j < 3; j++)
		{
			tri[j] = Endian.LittleLong(InIndices[i + j]);

			if (tri[j] >= numVerts) {
				throw BSPError::BadFaceSurfaceIndex(tri[j]);
			}
		}

		if ((tri[0] == tri[1]) || (tri[1] == tri[2]) || (tri[0] == tri[2]))
		{
			tri -= 3;
			badTriangles++;
		}
	}

	if (badTriangles)
	{
		out->indexes.resize(out->indexes.size() - badTriangles * 3);
	}

	for (i = 0; i < 3; i++) {
		out->cullInfo.plane.normal[i] = Endian.LittleFloat(InSurface->lightmapVecs[2][i]);
	}

	out->cullInfo.plane.distance = (float)castVector(out->vertices[0].xyz).dot(castVector(out->cullInfo.plane.normal));
	out->cullInfo.plane.type = PlaneTypeForNormal(out->cullInfo.plane.normal);
}

void BSPReader::ParseTriSurf(const BSPFile::fsurface_t* InSurface, const BSPFile::fvertice_t* InVertices, const int32_t* InIndices, const std::vector<BSPData::Shader>& shaders, Surface* out)
{
	uint32_t i, j;
	const size_t numVerts = Endian.LittleLong(InSurface->numVerts);
	const size_t numIndexes = Endian.LittleLong(InSurface->numIndexes);

	out->shader = &shaders[Endian.LittleLong(InSurface->shaderNum)];

	out->indexes.resize(numIndexes);
	out->vertices.resize(numVerts);

	// copy vertexes
	out->cullInfo.type = out->cullInfo.CULLINFO_BOX;
	ClearBounds(out->cullInfo.bounds[0], out->cullInfo.bounds[1]);
	InVertices += Endian.LittleLong(InSurface->firstVert);
	Vertice *OutVertices = out->vertices.data();
	for (i = 0; i < numVerts; i++)
	{
		for (j = 0; j < 3; j++)
		{
			OutVertices[i].xyz[j] = Endian.LittleFloat(InVertices[i].xyz[j]);
			OutVertices[i].normal[j] = Endian.LittleFloat(InVertices[i].normal[j]);
		}

		AddPointToBounds(out->vertices[i].xyz, out->cullInfo.bounds[0], out->cullInfo.bounds[1]);

		for (j = 0; j < 2; j++)
		{
			OutVertices[i].st[j] = Endian.LittleFloat(InVertices[i].st[j]);
			OutVertices[i].lightmap[j] = Endian.LittleFloat(InVertices[i].lightmap[j]);
		}

		out->vertices[i].color[0] = InVertices[i].color[0];
		out->vertices[i].color[1] = InVertices[i].color[1];
		out->vertices[i].color[2] = InVertices[i].color[2];
		out->vertices[i].color[3] = InVertices[i].color[3];
		//color[0] = InVertices[i].color[0];
		//color[1] = InVertices[i].color[1];
		//color[2] = InVertices[i].color[2];
		//color[3] = InVertices[i].color[3] / 255.0f;

		//ColorShiftLightingFloats(color, out->vertices[i].color, 1.0f / 255.0f);
	}

	// copy triangles
	size_t badTriangles = 0;
	uint32_t* tri;
	InIndices += Endian.LittleLong(InSurface->firstIndex);
	for (i = 0, tri = out->indexes.data(); i < numIndexes; i += 3, tri += 3)
	{
		for (j = 0; j < 3; j++)
		{
			tri[j] = Endian.LittleLong(InIndices[i + j]);

			if (tri[j] >= numVerts) {
				throw BSPError::BadFaceSurfaceIndex(tri[j]);
			}
		}

		if ((tri[0] == tri[1]) || (tri[1] == tri[2]) || (tri[0] == tri[2]))
		{
			tri -= 3;
			badTriangles++;
		}
	}

	if (badTriangles)
	{
		out->indexes.resize(out->indexes.size() - badTriangles * 3);
	}
}

void BSPReader::LoadSurfaces(
	const BSPFile::GameLump* SurfacesLump,
	const BSPFile::GameLump* Vertices,
	BSPFile::GameLump* Indices,
	const std::vector<BSPData::Shader>& shaders,
	std::vector<BSPData::Surface>& surfaces
)
{
	enum SurfaceType
	{
		MST_BAD,
		MST_PLANAR,
		MST_PATCH,
		MST_TRIANGLE_SOUP,
		MST_FLARE
	};

	if (SurfacesLump->length % sizeof(BSPFile::fsurface_t)) {
		throw BSPError::FunnyLumpSize("surfaces");
	}

	if (Vertices->length % sizeof(BSPFile::fvertice_t)) {
		throw BSPError::FunnyLumpSize("vertices");
	}

	if (Indices->length % sizeof(int32_t)) {
		throw BSPError::FunnyLumpSize("indices");
	}

	const size_t count = SurfacesLump->length / sizeof(BSPFile::fsurface_t);
	if (count)
	{
		surfaces.resize(count);

		const BSPFile::fsurface_t* in = (BSPFile::fsurface_t*)SurfacesLump->buffer;
		Surface* out = surfaces.data();

		const BSPFile::fvertice_t* InVerts = (BSPFile::fvertice_t*)Vertices->buffer;
		int32_t* InIndexes = (int32_t*)Indices->buffer;

		for (size_t i = 0; i < count; i++, in++, out++)
		{
			out->lightmapNum = Endian.LittleLong(in->lightmapNum);
			out->lightmapX = Endian.LittleLong(in->lightmapX);
			out->lightmapY = Endian.LittleLong(in->lightmapY);
			out->lightmapWidth = Endian.LittleLong(in->lightmapWidth);
			out->lightmapHeight = Endian.LittleLong(in->lightmapHeight);
			EndianHelpers::LittleVector(Endian, in->lightmapOrigin, out->lightmapOrigin);
			EndianHelpers::LittleVector(Endian, in->lightmapVecs[0], out->lightmapVecs[0]);
			EndianHelpers::LittleVector(Endian, in->lightmapVecs[1], out->lightmapVecs[1]);
			EndianHelpers::LittleVector(Endian, in->lightmapVecs[2], out->lightmapVecs[2]);

			switch (Endian.LittleLong(in->surfaceType))
			{
			case MST_PATCH:
				ParseMesh(in, InVerts, shaders, out);
				break;
			case MST_TRIANGLE_SOUP:
				ParseTriSurf(in, InVerts, InIndexes, shaders, out);
				break;
			case MST_PLANAR:
				ParseFace(in, InVerts, InIndexes, shaders, out);
				break;
			}

			out->CalculateCentroid();
		}
	}
}

void BSPReader::LoadPlanes(const BSPFile::GameLump* GameLump, std::vector<BSPData::Plane>& planes)
{
	if (GameLump->length % sizeof(BSPFile::fplane_t)) {
		throw BSPError::FunnyLumpSize("planes");
	}

	const size_t count = GameLump->length / sizeof(BSPFile::fplane_t);
	if (count)
	{
		planes.resize(count);

		const BSPFile::fplane_t* in = (BSPFile::fplane_t*)GameLump->buffer;
		Plane* out = planes.data();

		for (size_t i = 0; i < count; i++, in++, out++)
		{
			uint8_t Bits = 0;
			for (int32_t j = 0; j < 3; j++)
			{
				out->normal[j] = Endian.LittleFloat(in->normal[j]);
				if (out->normal[j] < 0)
				{
					Bits |= 1 << j;
				}
			}

			out->distance = Endian.LittleFloat(in->dist);
			out->type = PlaneTypeForNormal(out->normal);
			out->signBits = Bits;
		}
	}
}

void BSPReader::LoadSideEquations(const BSPFile::GameLump* GameLump, std::vector<BSPData::SideEquation>& sideEquations)
{
	if (GameLump->length % sizeof(BSPFile::fsideequation_t)) {
		throw BSPError::FunnyLumpSize("sideEquations");
	}

	const size_t count = GameLump->length / sizeof(BSPFile::fsideequation_t);
	if (count)
	{
		sideEquations.resize(count);

		const BSPFile::fsideequation_t* in = (BSPFile::fsideequation_t*)GameLump->buffer;
		SideEquation* out = sideEquations.data();

		for (size_t i = 0; i < count; i++, in++, out++)
		{
			for (int32_t j = 0; j < 4; j++)
			{
				out->sEq[j] = Endian.LittleFloat(in->seq[j]);
				out->tEq[j] = Endian.LittleFloat(in->teq[j]);
			}
		}
	}
}

void BSPReader::LoadBrushSides(
	const BSPFile::GameLump* GameLump,
	const std::vector<BSPData::Shader>& shaders,
	const std::vector<BSPData::Plane>& planes,
	const std::vector<BSPData::SideEquation>& sideEquations,
	std::vector<BSPData::BrushSide>& brushSides
)
{
	if (GameLump->length % sizeof(BSPFile::fbrushSide_t)) {
		throw BSPError::FunnyLumpSize("brushSides");
	}

	const size_t count = GameLump->length / sizeof(BSPFile::fbrushSide_t);
	if (count)
	{
		brushSides.resize(count);

		const BSPFile::fbrushSide_t* in = (BSPFile::fbrushSide_t*)GameLump->buffer;
		BrushSide* out = brushSides.data();

		for (size_t i = 0; i < count; i++, in++, out++)
		{
			const uint32_t num = Endian.LittleLong(in->planeNum);
			out->plane = &planes[num];
			out->shader = &shaders[Endian.LittleLong(in->shaderNum)];
			out->surfaceFlags = out->shader->surfaceFlags;

			const uint32_t eqNum = Endian.LittleLong(in->equationNum);
			if (eqNum)
			{
				out->Eq = &sideEquations[eqNum];
			}
			else
			{
				out->Eq = NULL;
			}
		}
	}
}

void BSPReader::LoadBrushes(
	const BSPFile::GameLump* GameLump,
	const std::vector<BSPData::Shader>& shaders,
	const std::vector<BSPData::BrushSide>& brushSides,
	std::vector<BSPData::Brush>& brushes
)
{
	if (GameLump->length % sizeof(BSPFile::fbrush_t)) {
		throw BSPError::FunnyLumpSize("brushes");
	}

	const size_t count = GameLump->length / sizeof(BSPFile::fbrush_t);
	if (count)
	{
		brushes.resize(count);

		const BSPFile::fbrush_t* in = (BSPFile::fbrush_t*)GameLump->buffer;
		Brush* out = brushes.data();

		for (size_t i = 0; i < count; i++, in++, out++)
		{
			out->sides = &brushSides[Endian.LittleLong(in->firstSide)];
			out->numsides = Endian.LittleLong(in->numSides);

			out->shader = &shaders[Endian.LittleLong(in->shaderNum)];

			out->contents = out->shader->contentFlags;
			out->parent = NULL;

			BoundBrush(out);
		}
	}
}

void BSPReader::LoadLeafs(
	const BSPFile::GameLump* GameLump,
	std::vector<BSPData::Leaf>& leafs,
	std::vector<BSPData::Area>& areas,
	std::vector<uintptr_t>& areaPortals,
	uint32_t& numClusters,
	uint32_t& numAreas
)
{
	numClusters = 0;
	numAreas = 0;

	if (GameLump->length % sizeof(BSPFile::fleaf_t)) {
		throw BSPError::FunnyLumpSize("leafs");
	}

	const size_t count = GameLump->length / sizeof(BSPFile::fleaf_t);
	if (count)
	{
		leafs.resize(count);

		const BSPFile::fleaf_t* in = (BSPFile::fleaf_t*)GameLump->buffer;
		Leaf* out = leafs.data();

		for (size_t i = 0; i < count; ++i, ++in, ++out)
		{
			out->cluster = Endian.LittleLong(in->cluster);
			out->area = Endian.LittleLong(in->area);
			out->firstLeafBrush = Endian.LittleLong(in->firstLeafBrush);
			out->numLeafBrushes = Endian.LittleLong(in->numLeafBrushes);
			out->firstLeafSurface = Endian.LittleLong(in->firstLeafSurface);
			out->numLeafSurfaces = Endian.LittleLong(in->numLeafSurfaces);
			out->firstLeafTerrain = Endian.LittleLong(in->firstTerraPatch);
			out->numLeafTerrains = Endian.LittleLong(in->numTerraPatches);

			if (out->cluster > 0 && (uint32_t)out->cluster >= numClusters) numClusters = out->cluster + 1;
			if (out->area > 0 && (uint32_t)out->area >= numAreas) numAreas = out->area + 1;
		}
	}

	areas.resize(numAreas);
	areaPortals.resize(numAreas * numAreas);
}

void BSPReader::LoadLeafsOld(
	const BSPFile::GameLump* GameLump,
	std::vector<BSPData::Leaf>& leafs,
	std::vector<BSPData::Area>& areas,
	std::vector<uintptr_t>& areaPortals,
	uint32_t& numClusters,
	uint32_t& numAreas
)
{
	numClusters = 0;
	numAreas = 0;

	if (GameLump->length % sizeof(BSPFile::fleaf_t)) {
		throw BSPError::FunnyLumpSize("leafs_ver17");
	}

	const size_t count = GameLump->length / sizeof(BSPFile::fleaf_ver17_t);
	if (count)
	{
		leafs.resize(count);

		const BSPFile::fleaf_ver17_t* in = (BSPFile::fleaf_ver17_t*)GameLump->buffer;
		Leaf* out = leafs.data();

		for (size_t i = 0; i < count; ++i, ++in, ++out)
		{
			out->cluster = Endian.LittleLong(in->cluster);
			out->area = Endian.LittleLong(in->area);
			out->firstLeafBrush = Endian.LittleLong(in->firstLeafBrush);
			out->numLeafBrushes = Endian.LittleLong(in->numLeafBrushes);
			out->firstLeafSurface = Endian.LittleLong(in->firstLeafSurface);
			out->numLeafSurfaces = Endian.LittleLong(in->numLeafSurfaces);

			if (out->cluster > 0 && (uint32_t)out->cluster >= numClusters) numClusters = out->cluster + 1;
			if (out->area > 0 && (uint32_t)out->area >= numAreas) numAreas = out->area + 1;
		}
	}

	areas.resize(numAreas);
	areaPortals.resize(numAreas * numAreas);
}

void BSPReader::LoadLeafsBrushes(const BSPFile::GameLump* GameLump, std::vector<uintptr_t>& leafBrushes)
{
	if (GameLump->length % sizeof(uint32_t)) {
		throw BSPError::FunnyLumpSize("leafBrushes");
	}

	const size_t count = GameLump->length / sizeof(uint32_t);
	if (count)
	{
		leafBrushes.resize(count);

		uint32_t* in = (uint32_t*)GameLump->buffer;

		for (size_t i = 0; i < count; ++i) {
			leafBrushes[i] = Endian.LittleLong(in[i]);
		}
	}
}

void BSPReader::LoadLeafSurfaces(const BSPFile::GameLump* GameLump, std::vector<uintptr_t>& leafSurfaces)
{
	if (GameLump->length % sizeof(uint32_t)) {
		throw BSPError::FunnyLumpSize("leafSurfaces");
	}

	const size_t count = GameLump->length / sizeof(uint32_t);
	if (count)
	{
		leafSurfaces.reserve(count);

		const uint32_t* in = (uint32_t*)GameLump->buffer;

		for (size_t i = 0; i < count; ++i) {
			leafSurfaces.push_back(Endian.LittleLong(in[i]));
		}
	}
}

void BSPReader::LoadNodes(
	const BSPFile::GameLump* GameLump,
	const std::vector<BSPData::Plane>& planes,
	std::vector<BSPData::Node>& nodes
)
{
	if (GameLump->length % sizeof(BSPFile::fnode_t)) {
		throw BSPError::FunnyLumpSize("nodes");
	}

	const size_t count = GameLump->length / sizeof(BSPFile::fnode_t);
	if (count)
	{
		nodes.resize(count);

		BSPFile::fnode_t* in = (BSPFile::fnode_t*)GameLump->buffer;
		Node* out = nodes.data();

		for (size_t i = 0; i < count; ++i, ++in, ++out)
		{
			out->plane = &planes[Endian.LittleLong(in->planeNum)];

			for (int j = 0; j < 2; ++j) {
				out->children[j] = Endian.LittleLong(in->children[j]);
			}
		}
	}
}

void BSPReader::LoadVisibility(const BSPFile::GameLump* GameLump, std::vector<uint8_t>& visibility, uint32_t& numClusters, uint32_t& clusterBytes)
{
	const size_t count = GameLump->length;
	if (!count)
	{
		visibility.resize(clusterBytes);
		return;
	}

	visibility.resize(count);
	numClusters = Endian.LittleLong(((const uint32_t*)GameLump->buffer)[0]);
	clusterBytes = Endian.LittleLong(((const uint32_t*)GameLump->buffer)[1]);
	memcpy(visibility.data(), (uint8_t*)GameLump->buffer + VIS_HEADER, count - VIS_HEADER);
}

void BSPReader::LoadSubmodels(
	const BSPFile::GameLump* GameLump,
	const std::vector<BSPData::Surface>& surfaces,
	std::vector<uintptr_t>& leafBrushes,
	std::vector<uintptr_t>& leafSurfaces,
	std::vector<BSPData::Model>& brushModels
)
{
	if (GameLump->length % sizeof(BSPFile::fbmodel_t)) {
		throw BSPError::FunnyLumpSize("submodels");
	}

	const size_t count = GameLump->length / sizeof(BSPFile::fbmodel_t);
	if (count)
	{
		brushModels.resize(count);

		const BSPFile::fbmodel_t* in = (BSPFile::fbmodel_t*)GameLump->buffer;
		Model* out = brushModels.data();

		size_t numLeafBrushes = 0;
		size_t numLeafSurfaces = 0;
		for (size_t i = 0; i < count; i++)
		{
			numLeafBrushes += Endian.LittleLong(in[i].numBrushes);
			numLeafSurfaces += Endian.LittleLong(in[i].numSurfaces);
		}

		size_t startLeafBrush = leafBrushes.size();
		size_t startLeafSurf = leafSurfaces.size();

		if(numLeafBrushes) {
			leafBrushes.resize(startLeafBrush + numLeafBrushes);
		}

		if(numLeafSurfaces) {
			leafSurfaces.resize(startLeafSurf + numLeafSurfaces);
		}

		for (size_t i = 0; i < count; i++, in++, out++)
		{
			const uint32_t numSurfaces = Endian.LittleLong(in->numSurfaces);
			const uint32_t numBrushes = Endian.LittleLong(in->numBrushes);
			const uint32_t firstSurface = Endian.LittleLong(in->firstSurface);
			const uint32_t firstBrush = Endian.LittleLong(in->firstBrush);

			// spread the mins / maxs by a pixel
			EndianHelpers::LittleVector(Endian, in->mins, out->bounds[0]);
			EndianHelpers::LittleVector(Endian, in->maxs, out->bounds[1]);
			castVector(out->bounds[0]) -= Vector3(1, 1, 1);
			castVector(out->bounds[1]) += Vector3(1, 1, 1);

			out->numSurfaces = numSurfaces;
			if (out->numSurfaces) {
				out->surface = &surfaces[firstSurface];
			}
			else {
				out->surface = nullptr;
			}

			out->leaf.numLeafBrushes = numBrushes;
			if (out->leaf.numLeafBrushes)
			{
				uintptr_t* indexes = &leafBrushes[startLeafBrush];
				out->leaf.firstLeafBrush = startLeafBrush;
				for (uintptr_t j = 0; j < out->leaf.numLeafBrushes; j++) {
					indexes[j] = firstBrush + j;
				}
			}
			else {
				out->leaf.firstLeafBrush = -1;
			}

			out->leaf.numLeafSurfaces = numSurfaces;
			if (out->leaf.numLeafSurfaces)
			{
				uintptr_t* indexes = &leafSurfaces[startLeafSurf];
				out->leaf.firstLeafSurface = startLeafSurf;
				for (uintptr_t j = 0; j < out->leaf.numLeafSurfaces; j++) {
					indexes[j] = firstSurface + j;
				}
			}
			else {
				out->leaf.firstLeafSurface = -1;
			}

			startLeafSurf += out->leaf.numLeafSurfaces;
			startLeafBrush += out->leaf.numLeafBrushes;
		}
	}
}

void BSPReader::LoadEntityString(const BSPFile::GameLump* GameLump, char*& entityString, size_t& entityStringLength)
{
	if(GameLump->length)
	{
		entityString = new char[GameLump->length];
		entityStringLength = GameLump->length;
		memcpy(entityString, GameLump->buffer, entityStringLength);
	}
}

void BSPReader::LoadSphereLights(const BSPFile::GameLump* GameLump, std::vector<BSPData::SphereLight>& lights)
{
	if (GameLump->length % sizeof(BSPFile::fsphereLight_t)) {
		throw BSPError::FunnyLumpSize("sphereLights");
	}

	size_t NumLights = GameLump->length / sizeof(BSPFile::fsphereLight_t);
	if (NumLights)
	{
		lights.resize(NumLights);

		const BSPFile::fsphereLight_t* in = (BSPFile::fsphereLight_t *)GameLump->buffer;
		SphereLight* out = lights.data();

		for (size_t i = 0; i < NumLights; in++, out++, i++)
		{
			EndianHelpers::LittleVector(Endian, in->origin, out->origin);
			EndianHelpers::LittleVector(Endian, in->color, out->color);
			EndianHelpers::LittleVector(Endian, in->spot_dir, out->spotDirection);

			out->spotRadiusByDistance = Endian.LittleFloat(in->spot_radiusbydistance);
			out->intensity = Endian.LittleFloat(in->intensity);
			out->bSpotLight = Endian.LittleLong(in->spot_light);
			out->bNeedsTrace = Endian.LittleLong(in->needs_trace);
		}
	}
}

void BSPReader::LoadStaticModelDefs(const BSPFile::GameLump* GameLump, std::vector<BSPData::StaticModel>& staticModels)
{
	if (GameLump->length % sizeof(BSPFile::fstaticModel_t)) {
		throw BSPError::FunnyLumpSize("planes");
	}

	const size_t NumStaticModels = GameLump->length / sizeof(BSPFile::fstaticModel_t);
	if (NumStaticModels > 0)
	{
		staticModels.resize(NumStaticModels);

		const BSPFile::fstaticModel_t* in = (BSPFile::fstaticModel_t *)GameLump->buffer;
		StaticModel* out = staticModels.data();

		for (size_t i = 0; i < NumStaticModels; in++, out++, i++)
		{
			out->visCount = 0;
			EndianHelpers::LittleVector(Endian, in->angles, out->angles);
			EndianHelpers::LittleVector(Endian, in->origin, out->origin);
			out->scale = Endian.LittleFloat(in->scale);
			out->firstVertexData = Endian.LittleLong(in->firstVertexData);
			out->numVertexData = Endian.LittleLong(in->numVertexData);
			out->modelName = CanonicalModelName(in->model);
		}
	}
}

void BSPReader::UnpackTerraPatch(const BSPFile::fterrainPatch_t* Packed, TerrainPatch* Unpacked, const std::vector<BSPData::Shader>& shaders) const
{
	int i;
	union {
		int16_t v;
		uint8_t b[2];
	} flags;

	Unpacked->drawInfo.triHead = 0;
	Unpacked->drawInfo.triTail = 0;
	Unpacked->drawInfo.mergeHead = 0;
	Unpacked->drawInfo.vertHead = 0;
	Unpacked->drawInfo.numTris = 0;
	Unpacked->drawInfo.numVerts = 0;

	Unpacked->bByDirty = false;
	Unpacked->visCountCheck = 0;
	Unpacked->visCountDraw = 0;
	Unpacked->distRecalc = 0;

	if (Packed->lmapScale <= 0)
	{
		// FIXME: Throw?
		return;
	}

	Unpacked->drawInfo.lmapStep = (float)(64 / Packed->lmapScale);
	Unpacked->drawInfo.lmapSize = (Packed->lmapScale * 8) | 1;
	Unpacked->s = ((float)Packed->s + 0.5f) / LIGHTMAP_SIZE;
	Unpacked->t = ((float)Packed->t + 0.5f) / LIGHTMAP_SIZE;

	Unpacked->drawInfo.lmapData = NULL;

	memcpy(Unpacked->texCoord, Packed->texCoord, sizeof(Unpacked->texCoord));
	Unpacked->x0 = (float)((int32_t)Packed->x << 6);
	Unpacked->y0 = (float)((int32_t)Packed->y << 6);
	Unpacked->z0 = Endian.LittleShort(Packed->iBaseHeight);
	Unpacked->shader = &shaders[Endian.LittleShort(Packed->iShader)];
	Unpacked->north = Endian.LittleShort(Packed->iNorth);
	Unpacked->east = Endian.LittleShort(Packed->iEast);
	Unpacked->south = Endian.LittleShort(Packed->iSouth);
	Unpacked->west = Endian.LittleShort(Packed->iWest);

	for (i = 0; i < 63; i++)
	{
		flags.v = Packed->varTree[0][i].flags;
		flags.b[1] &= 7;
		Unpacked->varTree[0][i].variance = Endian.LittleShort(flags.v);
		Unpacked->varTree[0][i].s.flags = Endian.LittleShort(Packed->varTree[0][i].flags) >> 12;

		flags.v = Packed->varTree[1][i].flags;
		flags.b[1] &= 7;
		Unpacked->varTree[1][i].variance = Endian.LittleShort(flags.v);
		Unpacked->varTree[1][i].s.flags = Endian.LittleShort(Packed->varTree[1][i].flags) >> 12;
	}

	memcpy(Unpacked->heightmap, Packed->heightmap, sizeof(Unpacked->heightmap));
	Unpacked->zmax = 0;
	Unpacked->flags = Packed->flags;

	for (i = 0; i < sizeof(Unpacked->heightmap); i++)
	{
		if (Unpacked->zmax < Unpacked->heightmap[i])
		{
			Unpacked->zmax = Unpacked->heightmap[i];
		}
	}

	Unpacked->frameCount = 0;
	Unpacked->zmax += Unpacked->zmax;
}

void BSPReader::LoadTerrain(const BSPFile::GameLump* GameLump, const std::vector<BSPData::Shader>& shaders, std::vector<BSPData::TerrainPatch>& terrainPatches)
{
	if (GameLump->length % sizeof(BSPFile::fterrainPatch_t)) {
		throw BSPError::FunnyLumpSize("terrain");
	}

	const size_t count = GameLump->length / sizeof(BSPFile::fterrainPatch_t);
	if (count > 0)
	{
		terrainPatches.resize(count);

		BSPFile::fterrainPatch_t* in = (BSPFile::fterrainPatch_t *)GameLump->buffer;
		TerrainPatch* out = terrainPatches.data();

		for (size_t i = 0; i < count; in++, out++, i++) {
			UnpackTerraPatch(in, out, shaders);
		}
	}
}

void BSPReader::LoadTerrainIndexes(
	const BSPFile::GameLump* GameLump,
	const std::vector<BSPData::TerrainPatch>& terrainPatches,
	std::vector<const BSPData::TerrainPatch*>& leafTerrains
)
{
	if (GameLump->length % sizeof(uint16_t)) {
		throw BSPError::FunnyLumpSize("terrainIndexes");
	}

	const size_t count = GameLump->length / sizeof(uint16_t);
	if (count > 0)
	{
		leafTerrains.resize(count);

		const uint16_t* in = (uint16_t*)GameLump->buffer;

		for (size_t i = 0; i < count; ++i) {
			leafTerrains[i] = &terrainPatches[Endian.LittleShort(in[i])];
		}
	}
}

void BSPReader::FloodArea(std::vector<BSPData::Area>& areas, const std::vector<uintptr_t>& areaPortals, uint32_t numAreas, uint32_t areaNum, uint32_t floodNum, uint32_t& floodValid)
{
	Area* area = &areas[areaNum];

	if(area->floodValid == floodValid)
	{
		if (area->floodNum == floodNum) {
			return;
		}

		throw BSPError::RefloodedArea(areaNum, floodNum);
	}

	area->floodNum = floodNum;
	area->floodValid = floodValid;
	const uintptr_t* con = &areaPortals[areaNum * numAreas];
	for (uint32_t i = 0; i < numAreas; ++i)
	{
		if (con[i] > 0) {
			FloodArea(areas, areaPortals, numAreas, i, floodNum, floodValid);
		}
	}
}

void BSPReader::FloodAreaConnections(std::vector<BSPData::Area>& areas, const std::vector<uintptr_t>& areaPortals, uint32_t numAreas)
{
	uint32_t floodValid = 1;
	uint32_t floodNum = 0;

	for (uint32_t i = 0; i < areas.size(); ++i)
	{
		Area* area = &areas[i];
		if (area->floodValid == floodValid) {
			continue;
		}

		++floodNum;
		FloodArea(areas, areaPortals, numAreas, i, floodNum, floodValid);
	}
}

void BSPReader::BoundBrush(Brush* Brush)
{
	Brush->bounds[0][0] = -Brush->sides[0].plane->distance;
	Brush->bounds[1][0] = Brush->sides[1].plane->distance;

	Brush->bounds[0][1] = -Brush->sides[2].plane->distance;
	Brush->bounds[1][1] = Brush->sides[3].plane->distance;

	Brush->bounds[0][2] = -Brush->sides[4].plane->distance;
	Brush->bounds[1][2] = Brush->sides[5].plane->distance;
}

void BSPReader::ColorShiftLightingFloats(float in[4], float out[4], float scale)
{
	float	r, g, b;

	r = in[0] * scale;
	g = in[1] * scale;
	b = in[2] * scale;

	// normalize by color instead of saturating to white
	if (r > 1 || g > 1 || b > 1) {
		float	max;

		max = r > g ? r : g;
		max = max > b ? max : b;
		r = r / max;
		g = g / max;
		b = b / max;
	}

	out[0] = r;
	out[1] = g;
	out[2] = b;
	out[3] = in[3];
}

void BSPReader::ColorShiftLightingFloats3(vec3_t in, vec3_t out, float scale)
{
	float	r, g, b;

	r = in[0] * scale;
	g = in[1] * scale;
	b = in[2] * scale;

	// normalize by color instead of saturating to white
	if (r > 1 || g > 1 || b > 1) {
		float	max;

		max = r > g ? r : g;
		max = max > b ? max : b;
		r = r / max;
		g = g / max;
		b = b / max;
	}

	out[0] = r;
	out[1] = g;
	out[2] = b;
}

uint32_t BSPReader::LoadLump(const IFilePtr& file, BSPFile::flump_t* lump, BSPFile::GameLump* gameLump, size_t size)
{
	const uint32_t fileLength = Endian.LittleLong(lump->fileLength);
	gameLump->length = fileLength;

	if (fileLength)
	{
		gameLump->buffer = new uint8_t[fileLength];

		std::istream* stream = file->GetStream();

		const uint32_t fileOffset = Endian.LittleLong(lump->fileOffset);
		stream->seekg(fileOffset, stream->beg);
		stream->read((char*)gameLump->buffer, fileLength);

		if (size) {
			return (uint32_t)(fileLength / size);
		}
	}
	else {
		gameLump->buffer = nullptr;
	}

	return 0;
}

void BSPReader::CreateEntities(
	const char* entityString,
	size_t length,
	std::vector<class LevelEntity*>& entities,
	std::unordered_map<str, std::vector<class LevelEntity*>>& targetList
)
{
	Script script;

	script.Parse(entityString, length);

	while (script.TokenAvailable(true))
	{
		const char *token = script.GetToken(false);
		if (!strHelpers::icmp(token, "{"))
		{
			PropertyMap propertiesMap;

			while (script.TokenAvailable(true))
			{
				token = script.GetToken(true);
				if (!strHelpers::icmp(token, "}")) {
					break;
				}

				if(!token) {
					throw BSPError::UnexpectedEntityEOF();
				}

				str key = token;
				str value;

				strHelpers::tolower(key.begin(), key.end());
				//std::transform(key.begin(), key.end(), key.begin(), ::tolower);

				/*
				if (!strHelpers::icmp(token, "targetname"))
				{
					token = script.GetToken(false);
					propertiesMap.insert_or_assign("targetname", token);
				}
				else if (!strHelpers::icmp(token, "target"))
				{
					token = script.GetToken(false);
					propertiesMap.insert_or_assign("target", token);
				}
				else if (!strHelpers::icmp(token, "classname"))
				{
					token = script.GetToken(false);
					propertiesMap.insert_or_assign("classname", token);
				}
				else if (!strHelpers::icmp(token, "model"))
				{
					token = script.GetToken(false);
					propertiesMap.insert_or_assign("model", CanonicalModelName(token));
				}
				else if (!strHelpers::icmp(token, "origin"))
				{
					token = script.GetString(false);
					propertiesMap.insert_or_assign("origin", token);
				}
				else if (!strHelpers::icmp(token, "angles"))
				{
					token = script.GetString(false);
					propertiesMap.insert_or_assign("angles", token);
				}
				else if (!strHelpers::icmp(token, "angle"))
				{
					token = script.GetToken(false);
					propertiesMap.insert_or_assign("angles", "0 " + str(token) + " 0");
				}
				else
				*/
				{
					value.clear();

					while (script.TokenAvailable(false))
					{
						if (value.length()) value += " ";

						const char* tokval = script.GetToken(false);
						if (*tokval) {
							value += tokval;
						}
					}

					propertiesMap.insert_or_assign(std::move(key), value);
				}
			}

			/*
			const char* classname = "";
			int32_t spawnflags = 0;
			const char* targetname = "";
			const char* target = "";

			PropertyMap::iterator it = propertiesMap.find("classname");
			if (it != propertiesMap.end())
			{
				classname = it->second.c_str();
			}

			it = propertiesMap.find("spawnflags");
			if (it != propertiesMap.end())
			{
				spawnflags = stoi(it->second);
			}

			it = propertiesMap.find("targetname");
			if (it != propertiesMap.end())
			{
				targetname = it->second.c_str();
			}

			it = propertiesMap.find("target");
			if (it != propertiesMap.end())
			{
				target = it->second.c_str();
			}
			*/

			LevelEntity* ent = new LevelEntity(entities.size());

			// Set all (members) properties
			for (PropertyMap::iterator it = propertiesMap.begin(); it != propertiesMap.end(); ++it)
			{
				ent->SetPropertyDef(std::move(it->first), std::move(it->second));
			}

			if (ent->IsClassOf("worldspawn"))
			{
				// The worldspawn will always be called world
				ent->SetTargetName("world");
			}

			if (*ent->GetTargetName())
			{
				// Insert the entity to the list of named entities
				auto pair = targetList.insert({ str(ent->GetTargetName()), std::vector<LevelEntity*>() });
				pair.first->second.push_back(ent);
			}

			entities.push_back(ent);
		}
		else {
			throw BSPError::ExpectedInitBrace(*token);
		}
	}

	MOHPC_LOG(Info, "created %d entities", entities.size());
}

void BSPReader::MapBrushes(
	std::vector<BSPData::Brush>& brushes,
	const std::vector<BSPData::Surface>& surfaces,
	const std::vector<BSPData::Model>& brushModels,
	const std::vector<BSPData::Surface>& terrainSurfaces,
	std::vector<BSPData::SurfacesGroup*>& surfacesGroups
)
{
	size_t numParentedBrushes = 0;
	const size_t numBrushes = brushes.size();

	Brush* brushesList = brushes.data();

	// Connects brushes by finding touching brushes that share at least one same shader
	for (size_t b = 0; b < numBrushes; b++)
	{
		Brush* brush = &brushesList[b];
		brush->name = "brush" + std::to_string(b);
		for (size_t i = 0; i < numBrushes; i++)
		{
			Brush* brush2 = &brushesList[i];
			if (brush2 == brush || brush2->parent)
			{
				continue;
			}

			const Brush* parent = brush;

			// Checks for infinite parenting
			bool bInfiniteParent = false;
			do
			{
				if (parent == brush2)
				{
					bInfiniteParent = true;
					break;
				}
				parent = parent->parent;
			} while (parent != NULL);

			if (bInfiniteParent)
			{
				continue;
			}

			// Checks if the two brushes are touching
			if (BrushIsTouching(brush, brush2))
			{
				bool bFound = false;

				// Finds at least one valid brush side that matches the parent's brush sides
				for (size_t bs = 0; bs < brush->numsides; bs++)
				{
					const BrushSide* brushside = &brush->sides[bs];
					if (brushside->surfaceFlags & SURF_NODRAW)
					{
						continue;
					}

					for (size_t bs2 = 0; bs2 < brush2->numsides; bs2++)
					{
						const BrushSide* brushside2 = &brush2->sides[bs2];
						if (brushside2->surfaceFlags & SURF_NODRAW)
						{
							continue;
						}

						if (!strHelpers::icmp(brushside2->shader->shaderName.c_str(), brushside->shader->shaderName.c_str()))
						{
							// Found one that matches
							bFound = true;
							break;
						}
					}
					
					if (bFound)
					{
						break;
					}
				}

				if (bFound)
				{
					numParentedBrushes++;
					brush2->parent = brush;
				}
			}
		}
	}

	assert(brushModels.size());
	const Model* worldModel = &brushModels[0];

	// Map brushes
	size_t numMappedSurfaces = 0;
	size_t numUnmappedSurfaces = 0;
	size_t numValidBrushes = 0;
	size_t numSurfacesGroups = 0;
	bool bBrushHasMappedSurface = false;

	const size_t numSurfaces = worldModel->numSurfaces;
	const size_t numTerrainSurfaces = terrainSurfaces.size();
	const size_t numTotalSurfaces = numSurfaces + numTerrainSurfaces;
	bool *mappedSurfaces = new bool[numSurfaces]();

	std::unordered_map<const Brush*, SurfacesGroup*> brushToSurfaces(numBrushes);
	surfacesGroups.reserve(numTotalSurfaces);

	const Surface *surfacesList = surfaces.data();
	const Surface *terrainSurfacesList = terrainSurfaces.data();

	// Group surfaces with brushes
	for (size_t b = 0; b < numBrushes; b++)
	{
		Brush* brush = &brushesList[b];
		const Brush* rootbrush = brush;
		while (rootbrush->parent != NULL)
		{
			rootbrush = rootbrush->parent;
		}

		//const str& brushname = rootbrush->name;
		const_vec3p_t mins = brush->bounds[0];
		const_vec3p_t maxs = brush->bounds[1];

		auto it = brushToSurfaces.find(rootbrush);

		for (size_t k = 0; k < numSurfaces; k++)
		{
			if (!mappedSurfaces[k])
			{
				const Surface *surf = &worldModel->surface[k];

				if (!surf->IsPatch()
					&& surf->centroid[0] >= mins[0] && surf->centroid[0] <= maxs[0]
					&& surf->centroid[1] >= mins[1] && surf->centroid[1] <= maxs[1]
					&& surf->centroid[2] >= mins[2] && surf->centroid[2] <= maxs[2])
				{
					for (size_t s = 0; s < brush->numsides; s++)
					{
						const BrushSide* side = &brush->sides[s];
						if (!strHelpers::icmp(side->shader->shaderName.c_str(), surf->shader->shaderName.c_str()))
						{
							brush->surfaces.push_back(surf);

							SurfacesGroup *sg = nullptr;
							if (it == brushToSurfaces.end())
							{
								sg = new SurfacesGroup;
								sg->name = "surfacesgroup" + std::to_string(numSurfacesGroups); // std::to_string(numSurfacesGroups);
								surfacesGroups.push_back(sg);
								auto res = brushToSurfaces.emplace(rootbrush, sg);
								assert(res.second);
								if (res.second)
								{
									it = res.first;
								}
								numSurfacesGroups++;
							}
							else
							{
								sg = it->second;
							}

							sg->surfaces.push_back(surf);

							if (std::find(sg->brushes.begin(), sg->brushes.end(), brush) == sg->brushes.end())
							{
								sg->brushes.push_back(brush);
							}

							mappedSurfaces[k] = true;
							numMappedSurfaces++;
							bBrushHasMappedSurface = true;
							break;
						}
					}
				}
			}
		}

		if (bBrushHasMappedSurface)
		{
			bBrushHasMappedSurface = false;
			numValidBrushes++;
		}
	}

	// Add the LOD terrain
	SurfacesGroup *terSg = new SurfacesGroup;
	terSg->name = "lod_terrain";
	terSg->surfaces.resize(numTerrainSurfaces);

	for (size_t k = 0; k < numTerrainSurfaces; k++)
	{
		const Surface *surf = &terrainSurfaces[k];

		terSg->surfaces[k] = surf;
	}

	surfacesGroups.push_back(terSg);

	// Gather patches
	{
		std::vector<Patch> patchList;
		patchList.reserve(numSurfaces);

		for (size_t k = 0; k < numSurfaces; k++)
		{
			const Surface *surf = &worldModel->surface[k];
			if (surf->IsPatch())
			{
				patchList.push_back(surf);
			}
		}

		const size_t numPatches = patchList.size();
		for (size_t k = 0; k < numPatches; k++)
		{
			Patch* patch1 = &patchList[k];
			for (size_t l = 0; l < numPatches; l++)
			{
				if (l != k)
				{
					Patch* patch2 = &patchList[l];
					if (!patch1->HasParent(patch2) && patch2->IsTouching(patch1))
					{
						patch2->parent = patch1;
					}
				}
			}
		}

		size_t numGroupedPatches = 0;
		for (size_t k = 0; k < numPatches; k++)
		{
			Patch* patch = &patchList[k];
			Patch* rootPatch = patch->GetRoot();

			// Get the number of surfaces
			size_t numSurfaces = 0;
			for (Patch* p = patch; p; p = p->parent)
			{
				bool& isMappedSurface = mappedSurfaces[p->surface - worldModel->surface];
				if (!isMappedSurface)
				{
					numSurfaces++;
				}
			}

			if (numSurfaces > 0)
			{
				if (!rootPatch->surfaceGroup)
				{
					rootPatch->surfaceGroup = new SurfacesGroup;
					rootPatch->surfaceGroup->name = "meshpatch_grouped" + std::to_string(numGroupedPatches++); // std::to_string(numGroupedPatches++);
					surfacesGroups.push_back(rootPatch->surfaceGroup);
				}

				SurfacesGroup *sg = rootPatch->surfaceGroup;

				sg->surfaces.reserve(sg->surfaces.size() + numSurfaces);
				for (Patch* p = patch; p; p = p->parent)
				{
					bool& isMappedSurface = mappedSurfaces[p->surface - worldModel->surface];
					if (!isMappedSurface)
					{
						sg->surfaces.push_back(p->surface);
						isMappedSurface = true;
					}
				}
			}
		}
	}

	// Group unused surfaces
	for (size_t k = 0; k < numSurfaces; k++)
	{
		if (!mappedSurfaces[k])
		{
			const Surface *surf = &worldModel->surface[k];

			SurfacesGroup *sg = new SurfacesGroup;
			sg->name = "surfacesgroup_unmapped" + std::to_string(numUnmappedSurfaces); // std::to_string(numUnmappedSurfaces);
			sg->surfaces.push_back(surf);
			surfacesGroups.push_back(sg);

			numUnmappedSurfaces++;
		}
	}

	// Calculate the origin for each group of surfaces
	for (size_t i = 0; i < surfacesGroups.size(); i++)
	{
		SurfacesGroup *sg = surfacesGroups[i];

		Vector3 avg(0, 0, 0);
		size_t numVertices = 0;

		for (size_t k = 0; k < sg->surfaces.size(); k++)
		{
			const Surface* surf = sg->surfaces[k];

			for (size_t v = 0; v < surf->vertices.size(); v++)
			{
				const Vertice* vert = &surf->vertices[v];

				avg += castVector(vert->xyz);

				AddPointToBounds(surf->vertices[v].xyz, sg->bounds[0], sg->bounds[1]);
			}

			numVertices += surf->vertices.size();
		}

		castVector(sg->origin) = avg / (float)numVertices;
	}

	surfacesGroups.shrink_to_fit();
	delete[] mappedSurfaces;
}

uintptr_t BSP::PointLeafNum(const vec3r_t p)
{
	if (!nodes.size()) {
		return 0;
	}

	return PointLeafNum_r(p, 0);
}

uintptr_t BSP::PointLeafNum_r(const vec3r_t p, intptr_t num)
{
	float d;

	while (num >= 0)
	{
		const Node* node = nodes.data() + num;
		const Plane* plane = node->plane;

		if (plane->type < Plane::PLANE_NON_AXIAL) {
			d = p[plane->type] - plane->distance;
		} else {
			d = DotProduct(plane->normal, p) - plane->distance;
		}

		if (d < 0) {
			num = node->children[1];
		} else {
			num = node->children[0];
		}
	}

	return -1 - num;
}

Vertice::Vertice()
	: xyz{ 0 }
	, st{ 0 }
	, lightmap{ 0 }
	, normal{ 0 }
	, color{ 0 }
{
}

PatchPlane::PatchPlane()
	: plane{ 0 }
	, signbits{ 0 }
{
}

MOHPC_OBJECT_DEFINITION(BSPReader);
BSPReader::BSPReader()
{
}

BSPReader::~BSPReader()
{
}

Asset2Ptr BSPReader::read(const IFilePtr& file)
{
	std::istream* stream = file->GetStream();

	BSPFile::fheader_t Header;
	stream->read((char*)&Header, sizeof(Header));

	if (memcmp(Header.ident, BSP_IDENT, sizeof(Header.ident)) && memcmp(Header.ident, BSP_EXPANSIONS_IDENT, sizeof(Header.ident)))
	{
		MOHPC_LOG(Error, "'%s' has wrong header", file->getName().generic_string().c_str());
		throw BSPError::BadHeader((uint8_t*)Header.ident);
	}

	const uint32_t version = Endian.LittleLong(Header.version);
	if (version < BSP_BETA_VERSION || version > BSP_MAX_VERSION)
	{
		MOHPC_LOG(Error, "'%s' has wrong version number (%i should be between %i and %i)", version, BSP_BETA_VERSION, BSP_MAX_VERSION);
		throw BSPError::WrongVersion(version);
	}

	char* entityString = nullptr;
	size_t entityStringLength = 0;

	const BSPPtr bsp = BSPPtr(new BSP(file->getName()));

	ProfilableCode("shaders",
		[&]()
		{
			BSPFile::GameLump GameLump;

			LoadLump(file, &Header.lumps[LUMP_SHADERS], &GameLump, sizeof(BSPFile::fshader_t));
			LoadShaders(&GameLump, bsp->getShaders());
		});

	ProfilableCode("planes",
		[&]()
		{
			BSPFile::GameLump GameLump;

			LoadLump(file, &Header.lumps[LUMP_PLANES], &GameLump, sizeof(BSPFile::fplane_t));
			LoadPlanes(&GameLump, bsp->getPlanes());
		});

	ProfilableCode("lightmaps",
		[&]()
		{
			BSPFile::GameLump GameLump;

			LoadLump(file, &Header.lumps[LUMP_LIGHTMAPS], &GameLump, sizeof(uint8_t));
			LoadLightmaps(&GameLump, bsp->getLightmaps());
		});

	ProfilableCode("surfaces",
		[&]()
		{
			BSPFile::GameLump SurfacesLump;
			BSPFile::GameLump VerticesLump;
			BSPFile::GameLump IndexesLump;

			LoadLump(file, &Header.lumps[LUMP_SURFACES], &SurfacesLump, sizeof(BSPFile::fshader_t));
			LoadLump(file, &Header.lumps[LUMP_DRAWVERTS], &VerticesLump, sizeof(BSPFile::fvertice_t));
			LoadLump(file, &Header.lumps[LUMP_DRAWINDEXES], &IndexesLump, sizeof(int32_t));
			LoadSurfaces(&SurfacesLump, &VerticesLump, &IndexesLump, bsp->getShaders(), bsp->getSurfaces());
		});

	ProfilableCode("side equations",
		[&]()
		{
			BSPFile::GameLump GameLump;

			LoadLump(file, &Header.lumps[LUMP_SIDEEQUATIONS], &GameLump, 0);
			LoadSideEquations(&GameLump, bsp->getSideEquations());
		});

	ProfilableCode("brush sides",
		[&]()
		{
			BSPFile::GameLump GameLump;

			LoadLump(file, &Header.lumps[LUMP_BRUSHSIDES], &GameLump, 0);
			LoadBrushSides(&GameLump, bsp->getShaders(), bsp->getPlanes(), bsp->getSideEquations(), bsp->getBrushSides());
		});

	ProfilableCode("brushes",
		[&]()
		{
			BSPFile::GameLump GameLump;

			LoadLump(file, &Header.lumps[LUMP_BRUSHES], &GameLump, 0);
			LoadBrushes(&GameLump, bsp->getShaders(), bsp->getBrushSides(), bsp->getBrushes());
		});

	ProfilableCode("leaf brushes",
		[&]()
		{
			BSPFile::GameLump GameLump;

			LoadLump(file, &Header.lumps[LUMP_LEAFBRUSHES], &GameLump, 0);
			LoadLeafsBrushes(&GameLump, bsp->getLeafBrushes());
		});

	ProfilableCode("leaf surfaces",
		[&]()
		{
			BSPFile::GameLump GameLump;

			LoadLump(file, &Header.lumps[LUMP_LEAFSURFACES], &GameLump, 0);
			LoadLeafSurfaces(&GameLump, bsp->getLeafSurfaces());
		});

	ProfilableCode("leafs",
		[&]()
		{
			BSPFile::GameLump GameLump;

			LoadLump(file, &Header.lumps[LUMP_LEAFS], &GameLump, 0);
			uint32_t numClusters = 0;
			uint32_t numAreas = 0;
			if (version > BSP_BETA_VERSION) {
				LoadLeafs(&GameLump, bsp->getLeafs(), bsp->getAreas(), bsp->getAreaPortals(), numClusters, numAreas);
			}
			else {
				LoadLeafsOld(&GameLump, bsp->getLeafs(), bsp->getAreas(), bsp->getAreaPortals(), numClusters, numAreas);
			}

			bsp->setNumClusters(numClusters);
			bsp->setNumAreas(numAreas);
		});

	ProfilableCode("nodes",
		[&]()
		{
			BSPFile::GameLump GameLump;

			LoadLump(file, &Header.lumps[LUMP_NODES], &GameLump, 0);
			LoadNodes(&GameLump, bsp->getPlanes(), bsp->getNodes());
		});

	ProfilableCode("visibility",
		[&]()
		{
			BSPFile::GameLump GameLump;

			LoadLump(file, &Header.lumps[LUMP_VISIBILITY], &GameLump, 0);


			uint32_t numClusters = 0;
			uint32_t clusterBytes = 0;
			LoadVisibility(&GameLump, bsp->getVisibility(), numClusters, clusterBytes);

			if (numClusters) bsp->setNumClusters(numClusters);
		});

	ProfilableCode("models",
		[&]()
		{
			BSPFile::GameLump GameLump;

			LoadLump(file, &Header.lumps[LUMP_MODELS], &GameLump, 0);
			LoadSubmodels(&GameLump, bsp->getSurfaces(), bsp->getLeafBrushes(), bsp->getLeafSurfaces(), bsp->getBrushModels());
		});

	ProfilableCode("entities",
		[&]()
		{
			BSPFile::GameLump GameLump;

			LoadLump(file, &Header.lumps[LUMP_ENTITIES], &GameLump, 0);
			LoadEntityString(&GameLump, entityString, entityStringLength);
		});

	ProfilableCode("sphere lights",
		[&]()
		{
			BSPFile::GameLump GameLump;

			LoadLump(file, &Header.lumps[LUMP_SPHERELIGHTS], &GameLump, 0);
			LoadSphereLights(&GameLump, bsp->getSphereLights());
		});

	ProfilableCode("terrains",
		[&]()
		{
			BSPFile::GameLump GameLump;

			LoadLump(file, &Header.lumps[LUMP_TERRAIN], &GameLump, 0);
			LoadTerrain(&GameLump, bsp->getShaders(), bsp->getTerrainPatches());
		});

	ProfilableCode("terrain indexes",
		[&]()
		{
			BSPFile::GameLump GameLump;

			LoadLump(file, &Header.lumps[LUMP_TERRAININDEXES], &GameLump, 0);
			LoadTerrainIndexes(&GameLump, bsp->getTerrainPatches(), bsp->getLeafTerrains());
		});

	if (Header.version > BSP_BETA_VERSION)
	{
		ProfilableCode("static models",
			[&]()
			{
				BSPFile::GameLump GameLump;

				LoadLump(file, &Header.lumps[LUMP_STATICMODELDEF], &GameLump, 0);
				LoadStaticModelDefs(&GameLump, bsp->getStaticModels());
			});
	}

	ProfilableCode("generation of terrain surfaces",
		[&]()
		{
			CreateTerrainSurfaces(bsp->getTerrainPatches(), bsp->getTerrainSurfaces());
		});

	ProfilableCode("brush mapping",
		[&]()
		{
			MapBrushes(bsp->getBrushes(), bsp->getSurfaces(), bsp->getBrushModels(), bsp->getTerrainSurfaces(), bsp->getSurfacesGroups());
		});

	// FIXME: Should use an external class for creating entities
	ProfilableCode("creation of entities",
		[&]()
		{
			CreateEntities(entityString, entityStringLength, bsp->getEntities(), bsp->getTargetList());
		});

	MOHPC_LOG(Info, "Loaded map '%s', version %d", file->getName().generic_string().c_str(), version);

	return bsp;
}


std::vector<MOHPC::BSPData::Shader>& BSP::getShaders()
{
	return shaders;
}

std::vector<MOHPC::BSPData::Lightmap>& BSP::getLightmaps()
{
	return lightmaps;
}

std::vector<MOHPC::BSPData::Surface>& BSP::getSurfaces()
{
	return surfaces;
}

std::vector<MOHPC::BSPData::Plane>& BSP::getPlanes()
{
	return planes;
}

std::vector<MOHPC::BSPData::SideEquation>& BSP::getSideEquations()
{
	return sideEquations;
}

std::vector<MOHPC::BSPData::BrushSide>& BSP::getBrushSides()
{
	return brushSides;
}

std::vector<MOHPC::BSPData::Brush>& BSP::getBrushes()
{
	return brushes;
}

std::vector<MOHPC::BSPData::Node>& BSP::getNodes()
{
	return nodes;
}

std::vector<MOHPC::BSPData::Leaf>& BSP::getLeafs()
{
	return leafs;
}

std::vector<uintptr_t>& BSP::getLeafBrushes()
{
	return leafBrushes;
}

std::vector<uintptr_t>& BSP::getLeafSurfaces()
{
	return leafSurfaces;
}

std::vector<const BSPData::TerrainPatch*>& BSP::getLeafTerrains()
{
	return leafTerrains;
}

std::vector<MOHPC::BSPData::Area>& BSP::getAreas()
{
	return areas;
}

std::vector<uintptr_t>& BSP::getAreaPortals()
{
	return areaPortals;
}

std::vector<MOHPC::BSPData::Model>& BSP::getBrushModels()
{
	return brushModels;
}

std::vector<MOHPC::BSPData::SphereLight>& BSP::getSphereLights()
{
	return lights;
}

std::vector<MOHPC::BSPData::StaticModel>& BSP::getStaticModels()
{
	return staticModels;
}

std::vector<MOHPC::BSPData::TerrainPatch>& BSP::getTerrainPatches()
{
	return terrainPatches;
}

std::vector<MOHPC::BSPData::Surface>& BSP::getTerrainSurfaces()
{
	return terrainSurfaces;
}

std::vector<class LevelEntity*>& BSP::getEntities()
{
	return entities;
}

std::unordered_map<MOHPC::str, std::vector<class LevelEntity*>>& BSP::getTargetList()
{
	return targetList;
}

std::vector<MOHPC::BSPData::SurfacesGroup*>& BSP::getSurfacesGroups()
{
	return surfacesGroups;
}

std::vector<uint8_t>& BSP::getVisibility()
{
	return visibility;
}

void BSP::setNumClusters(uint32_t numClustersValue)
{
	numClusters = numClustersValue;
}

void BSP::setNumAreas(uint32_t numAreasValue)
{
	numAreas = numAreasValue;
}

BSPError::BadHeader::BadHeader(const uint8_t inHeader[4])
	: foundHeader{ inHeader[0], inHeader[1], inHeader[2], inHeader[3] }
{}

const uint8_t* BSPError::BadHeader::getHeader() const
{
	return foundHeader;
}

const char* BSPError::BadHeader::what() const noexcept
{
	return "Bad BSP header";
}

BSPError::WrongVersion::WrongVersion(uint32_t inVersion)
	: version(inVersion)
{

}

uint32_t BSPError::WrongVersion::getVersion() const
{
	return version;
}

const char* BSPError::WrongVersion::what() const noexcept
{
	return "Wrong BSP version";
}

BSPError::FunnyLumpSize::FunnyLumpSize(const char* inLumpName)
	: lumpName(inLumpName)
{}

const char* BSPError::FunnyLumpSize::getLumpName() const
{
	return lumpName;
}

const char* BSPError::FunnyLumpSize::what() const noexcept
{
	return "Funny lump size";
}

BSPError::BadMeshSize::BadMeshSize(int32_t inWidth, int32_t inHeight)
	: width(inWidth)
	, height(inHeight)
{
}

int32_t BSPError::BadMeshSize::getWidth() const
{
	return width;
}

int32_t BSPError::BadMeshSize::getHeight() const
{
	return height;
}

const char* BSPError::BadMeshSize::what() const noexcept
{
	return "Bad mesh size";
}

BSPError::BadFaceSurfaceIndex::BadFaceSurfaceIndex(uint32_t inIndex)
	: index(inIndex)
{}

uint32_t BSPError::BadFaceSurfaceIndex::getIndex() const
{
	return index;
}

const char* BSPError::BadFaceSurfaceIndex::what() const noexcept
{
	return "Invalid surface index";
}

BSPError::BadTerrainLightmapScale::BadTerrainLightmapScale(uint8_t inScale)
	: scale(inScale)
{}

uint8_t BSPError::BadTerrainLightmapScale::getScale() const
{
	return scale;
}

const char* BSPError::BadTerrainLightmapScale::what() const noexcept
{
	return "Bad terrain lightmap scale";
}

BSPError::RefloodedArea::RefloodedArea(uint32_t inAreaNum, uint32_t inFloodNum)
	: areaNum(inAreaNum)
	, floodNum(inFloodNum)
{}

uint32_t BSPError::RefloodedArea::getAreaNum() const
{
	return areaNum;
}

uint32_t BSPError::RefloodedArea::getFloodNum() const
{
	return floodNum;
}

const char* BSPError::RefloodedArea::what() const noexcept
{
	return "Trying to flood an area that was already flooded";
}

BSPError::ExpectedInitBrace::ExpectedInitBrace(char inChar)
	: c(inChar)
{
}

char BSPError::ExpectedInitBrace::getCharacter() const
{
	return c;
}

const char* BSPError::ExpectedInitBrace::what() const noexcept
{
	return "Expected an init brace '{'";
}
