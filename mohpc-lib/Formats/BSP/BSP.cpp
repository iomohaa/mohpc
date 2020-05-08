#include <Shared.h>
#include <MOHPC/Formats/BSP.h>
#include <MOHPC/Vector.h>
#include <MOHPC/Script.h>
#include <MOHPC/LevelEntity.h>
#include <MOHPC/Managers/FileManager.h>
#include <MOHPC/Managers/AssetManager.h>
#include <MOHPC/Managers/ShaderManager.h>
#include "BSP_Curve.h"
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

static void ProfilableCode(const char *profileName, std::function<void()> Lambda)
{
#ifdef _DEBUG
	auto start = std::chrono::system_clock().now();
	{
		Lambda();
	}
	auto end = std::chrono::system_clock().now();

	printf("%lf time (%s)\n", std::chrono::duration<double>(end - start).count(), profileName);
#else
	Lambda();
#endif
}

namespace MOHPC
{
	// little-endian "2015"
#define BSP_IDENT	(('5'<<24)+('1'<<16)+('0'<<8)+'2')
	// little-endian "EALA"
#define BSP_EXPANSIONS_IDENT (('A'<<24)+('L'<<16)+('A'<<8)+'E')

#define BSP_BETA_VERSION	18	// Beta Allied Assault
#define BSP_BASE_VERSION	19	// vanilla Allied Assault
#define BSP_VERSION			19	// current Allied Assault
#define BSP_MAX_VERSION		21	// MOH:BT

#define LUMP_FOGS				0
#define	LUMP_LIGHTGRID			0
	// new lump defines. lump numbers are different in mohaa
#define LUMP_SHADERS			0
#define LUMP_PLANES				1
#define LUMP_LIGHTMAPS			2
#define LUMP_SURFACES			3
#define LUMP_DRAWVERTS			4
#define LUMP_DRAWINDEXES		5
#define LUMP_LEAFBRUSHES		6
#define LUMP_LEAFSURFACES		7
#define LUMP_LEAFS				8
#define LUMP_NODES				9
#define LUMP_SIDEEQUATIONS		10
#define LUMP_BRUSHSIDES			11
#define LUMP_BRUSHES			12
/*
// FOG seems to be handled differently in MOHAA - no fog lump found yet
#define LUMP_FOGS				0
*/
#define LUMP_MODELS				13
#define LUMP_ENTITIES			14
#define LUMP_VISIBILITY			15
#define LUMP_LIGHTGRIDPALETTE	16
#define LUMP_LIGHTGRIDOFFSETS	17
#define LUMP_LIGHTGRIDDATA		18
#define LUMP_SPHERELIGHTS		19
#define LUMP_SPHERELIGHTVIS		20
#define LUMP_LIGHTDEFS			21
#define LUMP_TERRAIN			22
#define LUMP_TERRAININDEXES		23
#define LUMP_STATICMODELDATA	24
#define LUMP_STATICMODELDEF		25
#define LUMP_STATICMODELINDEXES	26
#define LUMP_DUMMY10			27

#define	HEADER_LUMPS		28

	struct BSP::GameLump
	{
		void* Buffer;
		size_t Length;
	};

	struct BSP::File_Lump
	{
		int32_t FileOffset;
		int32_t FileLength;
	};

	struct BSP::File_Header
	{
		int32_t ident;
		int32_t version;
		int32_t checksum;

		BSP::File_Lump lumps[HEADER_LUMPS];
	};

	struct BSP::File_Shader
	{
		char shader[64];
		int32_t surfaceFlags;
		int32_t contentFlags;
		int32_t subdivisions;
		char fenceMaskImage[64];
	};

	struct BSP::File_Surface
	{
		int32_t shaderNum;
		int32_t fogNum;
		int32_t surfaceType;
		int32_t firstVert;
		int32_t numVerts;
		int32_t firstIndex;
		int32_t numIndexes;
		int32_t lightmapNum;
		int32_t lightmapX, lightmapY;
		int32_t lightmapWidth, lightmapHeight;
		float lightmapOrigin[3];
		float lightmapVecs[3][3];
		int32_t patchWidth;
		int32_t patchHeight;
		float subdivisions;
	};

	struct BSP::File_Vertice
	{
		float xyz[3];
		float st[2];
		float lightmap[2];
		float normal[3];
		uint8_t color[4];
	};

	struct BSP::File_Plane
	{
		float normal[3];
		float dist;
	};

	struct BSP::File_SideEquation
	{
		float seq[4];
		float teq[4];
	};

	struct BSP::File_BrushSide
	{
		int32_t planeNum;
		int32_t shaderNum;
		int32_t equationNum;
	};

	struct BSP::File_Brush
	{
		int32_t firstSide;
		int32_t numSides;
		int32_t shaderNum;
	};

	struct BSP::File_BrushModel
	{
		float mins[3];
		float maxs[3];
		int32_t firstSurface;
		int32_t numSurfaces;
		int32_t firstBrush;
		int32_t numBrushes;
	};

	struct BSP::File_SphereLight
	{
		float origin[3];
		float color[3];
		float intensity;
		int leaf;
		int32_t needs_trace;
		int32_t spot_light;
		float spot_dir[3];
		float spot_radiusbydistance;
	};

	struct BSP::File_StaticModel
	{
		char model[128];
		float origin[3];
		float angles[3];
		float scale;
		int32_t firstVertexData;
		int32_t numVertexData;
	};

	struct BSP::File_TerrainPatch
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

	struct BSP::File_Node {
		int			planeNum;
		int			children[2];
		int			mins[3];
		int			maxs[3];
	};

	struct BSP::File_Leaf {
		int32_t cluster;
		int32_t area;
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
	struct BSP::File_Leaf_Ver17 {
		int32_t cluster;
		int32_t area;
		uint32_t mins[3];
		uint32_t maxs[3];
		uint32_t firstLeafSurface;
		uint32_t numLeafSurfaces;
		uint32_t firstLeafBrush;
		uint32_t numLeafBrushes;
		uint32_t firstTerraPatch;
		uint32_t numTerraPatches;
	};

	struct Patch
	{
		Patch* parent;
		const BSP::Surface* surface;
		Vector bounds[2];
		BSP::SurfacesGroup* surfaceGroup;

		Patch(const BSP::Surface* inSurface)
			: parent(nullptr)
			, surface(inSurface)
			, surfaceGroup(nullptr)
		{
			//const size_t numVertices = surface->GetNumVertices();
			//for (size_t k = 0; k < numVertices; k++)
			//{
			//	AddPointToBounds(surface->GetVertice(k)->xyz, bounds[0], bounds[1]);
			//}

			bounds[0] = surface->GetPatchCollide()->bounds[0];
			bounds[1] = surface->GetPatchCollide()->bounds[1];
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

const char* BSP::Brush::GetName() const
{
	return name.c_str();
}

const BSP::Shader* BSP::Brush::GetShader() const
{
	return shader;
}

int32_t BSP::Brush::GetContents() const
{
	return contents;
}

const MOHPC::Vector& BSP::Brush::GetMins() const
{
	return bounds[0];
}

const MOHPC::Vector& BSP::Brush::GetMaxs() const
{
	return bounds[1];
}

size_t BSP::Brush::GetNumSides() const
{
	return numsides;
}

BSP::BrushSide* BSP::Brush::GetSide(size_t Index) const
{
	return Index < numsides ? &sides[Index] : nullptr;
}

BSP::Brush* BSP::Brush::GetParent() const
{
	return parent;
}

BSP::PatchCollide::~PatchCollide()
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

size_t BSP::Lightmap::GetNumPixels() const
{
	return sizeof(color) / sizeof(color[0]);
}

size_t BSP::Lightmap::GetWidth() const
{
	return lightmapSize;
}

size_t BSP::Lightmap::GetHeight() const
{
	return lightmapSize;
}

void BSP::Lightmap::GetColor(size_t pixelNum, uint8_t(&out)[3]) const
{
	out[0] = color[pixelNum][0];
	out[1] = color[pixelNum][1];
	out[2] = color[pixelNum][2];
}

BSP::Surface::Surface()
{
	shader = nullptr;
	bIsPatch = false;
	pc = nullptr;
}

BSP::Surface::~Surface()
{
	if (pc)
	{
		delete pc;
	}
}

const BSP::Shader* BSP::Surface::GetShader() const
{
	return shader;
}

size_t BSP::Surface::GetNumVertices() const
{
	return vertices.size();
}

const BSP::Vertice *BSP::Surface::GetVertice(size_t index) const
{
	if (index < vertices.size())
	{
		return &vertices[index];
	}
	else
	{
		return nullptr;
	}
}


size_t BSP::Surface::GetNumIndexes() const
{
	return indexes.size();
}

size_t BSP::Surface::GetIndice(size_t index) const
{
	if (index < indexes.size())
	{
		return indexes[index];
	}
	else
	{
		return 0;
	}
}

int32_t BSP::Surface::GetLightmapNum() const
{
	return lightmapNum;
}

int32_t BSP::Surface::GetLightmapX() const
{
	return lightmapX;
}

int32_t BSP::Surface::GetLightmapY() const
{
	return lightmapY;
}

int32_t BSP::Surface::GetLightmapWidth() const
{
	return lightmapWidth;
}

int32_t BSP::Surface::GetLightmapHeight() const
{
	return lightmapHeight;
}

const Vector& BSP::Surface::GetLightmapOrigin() const
{
	return lightmapOrigin;
}

const Vector& BSP::Surface::GetLightmapVec(int32_t num) const
{
	if (num >= 0 && num < 3)
	{
		return lightmapVecs[num];
	}
	else
	{
		return vec_zero;
	}
}

const BSP::PatchCollide* BSP::Surface::GetPatchCollide() const
{
	return pc;
}

bool BSP::Surface::IsPatch() const
{
	return bIsPatch;
}

void BSP::Surface::CalculateCentroid()
{
	Vector avgVert = vec_zero;
	const size_t numVerts = vertices.size();
	const Vertice *verts = vertices.data();
	for (size_t v = 0; v < numVerts; v++)
	{
		const Vertice* pVert = &verts[v];
		avgVert += pVert->xyz;
	}

	centroid = avgVert / (float)numVerts;
}

Vector BSP::Brush::GetOrigin() const
{
	return (bounds[0] + bounds[1]) * 0.5f;
}

const str& BSP::SurfacesGroup::GetGroupName() const
{
	return name;
}

size_t BSP::SurfacesGroup::GetNumSurfaces() const
{
	return surfaces.size();
}

const BSP::Surface *BSP::SurfacesGroup::GetSurface(size_t index) const
{
	if (index < surfaces.size())
	{
		return surfaces.at(index);
	}
	else
	{
		return nullptr;
	}
}

const BSP::Surface* const *BSP::SurfacesGroup::GetSurfaces() const
{
	return surfaces.data();
}

size_t BSP::SurfacesGroup::GetNumBrushes() const
{
	return brushes.size();
}

const BSP::Brush *BSP::SurfacesGroup::GetBrush(size_t index) const
{
	if (index < brushes.size())
	{
		return brushes.at(index);
	}
	else
	{
		return nullptr;
	}
}

const BSP::Brush* const *BSP::SurfacesGroup::GetBrushes() const
{
	return brushes.data();
}

const Vector& BSP::SurfacesGroup::GetMinBound() const
{
	return bounds[0];
}

const Vector& BSP::SurfacesGroup::GetMaxBound() const
{
	return bounds[1];
}

const Vector& BSP::SurfacesGroup::GetOrigin() const
{
	return origin;
}

static bool BrushIsTouching(BSP::Brush* b1, BSP::Brush* b2)
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

CLASS_DEFINITION(BSP);

BSP::BSP()
{
	numClusters = 0;
	numAreas = 0;
	entityString = NULL;
	entityStringLength = 0;
}

BSP::~BSP()
{
	if (entityString)
	{
		delete[] entityString;
		entityStringLength = 0;
	}

	for (size_t i = 0; i < entities.size(); i++)
	{
		delete entities[i];
	}

	for (size_t i = 0; i < surfacesGroups.size(); i++)
	{
		delete surfacesGroups[i];
	}
}

bool BSP::Load()
{
	FilePtr file = GetFileManager()->OpenFile(GetFilename().c_str());
	if (!file)
	{
		return false;
	}

	std::istream* stream = file->GetStream();

	BSP::File_Header Header;
	stream->read((char*)&Header, sizeof(Header));

	if (Header.ident != BSP_IDENT && Header.ident != BSP_EXPANSIONS_IDENT)
	{
		return false;
	}

	if (Header.version < BSP_BETA_VERSION || Header.version > BSP_MAX_VERSION)
	{
		return false;
	}

	HashUpdate((uint8_t*)&Header, sizeof(Header));

	ProfilableCode("shaders",
	[&]()
	{
		BSP::GameLump GameLump;

		LoadLump(file, &Header.lumps[LUMP_SHADERS], &GameLump, sizeof(BSP::File_Shader));
		LoadShaders(&GameLump);
		FreeLump(&GameLump);
	});

	ProfilableCode("planes",
	[&]()
	{
		BSP::GameLump GameLump;

		LoadLump(file, &Header.lumps[LUMP_PLANES], &GameLump, sizeof(BSP::File_Plane));
		LoadPlanes(&GameLump);
		FreeLump(&GameLump);
	});

	ProfilableCode("lightmaps",
	[&]()
	{
		BSP::GameLump GameLump;

		LoadLump(file, &Header.lumps[LUMP_LIGHTMAPS], &GameLump, sizeof(uint8_t));
		LoadLightmaps(&GameLump);
		FreeLump(&GameLump);
	});

	ProfilableCode("surfaces",
	[&]()
	{
		BSP::GameLump SurfacesLump;
		BSP::GameLump VerticesLump;
		BSP::GameLump IndexesLump;

		LoadLump(file, &Header.lumps[LUMP_SURFACES], &SurfacesLump, sizeof(BSP::File_Shader));
		LoadLump(file, &Header.lumps[LUMP_DRAWVERTS], &VerticesLump, sizeof(BSP::File_Vertice));
		LoadLump(file, &Header.lumps[LUMP_DRAWINDEXES], &IndexesLump, sizeof(int32_t));
		LoadSurfaces(&SurfacesLump, &VerticesLump, &IndexesLump);
		FreeLump(&IndexesLump);
		FreeLump(&VerticesLump);
		FreeLump(&SurfacesLump);
	});

	ProfilableCode("side equations",
	[&]()
	{
		BSP::GameLump GameLump;

		LoadLump(file, &Header.lumps[LUMP_SIDEEQUATIONS], &GameLump, 0);
		LoadSideEquations(&GameLump);
		FreeLump(&GameLump);
	});

	ProfilableCode("brush sides",
	[&]()
	{
		BSP::GameLump GameLump;

		LoadLump(file, &Header.lumps[LUMP_BRUSHSIDES], &GameLump, 0);
		LoadBrushSides(&GameLump);
		FreeLump(&GameLump);
	});

	ProfilableCode("brushes",
	[&]()
	{
		BSP::GameLump GameLump;

		LoadLump(file, &Header.lumps[LUMP_BRUSHES], &GameLump, 0);
		LoadBrushes(&GameLump);
		FreeLump(&GameLump);
	});

	ProfilableCode("leaf brushes",
	[&]()
	{
		BSP::GameLump GameLump;

		LoadLump(file, &Header.lumps[LUMP_LEAFBRUSHES], &GameLump, 0);
		LoadLeafsBrushes(&GameLump);
		FreeLump(&GameLump);
	});

	ProfilableCode("leaf surfaces",
	[&]()
	{
		BSP::GameLump GameLump;

		LoadLump(file, &Header.lumps[LUMP_LEAFSURFACES], &GameLump, 0);
		LoadLeafSurfaces(&GameLump);
		FreeLump(&GameLump);
	});

	ProfilableCode("leafs",
	[&]()
	{
		BSP::GameLump GameLump;

		LoadLump(file, &Header.lumps[LUMP_LEAFS], &GameLump, 0);
		if (Header.version > BSP_BETA_VERSION) {
			LoadLeafs(&GameLump);
		}
		else {
			LoadLeafsOld(&GameLump);
		}
		FreeLump(&GameLump);
	});

	ProfilableCode("nodes",
	[&]()
	{
		BSP::GameLump GameLump;

		LoadLump(file, &Header.lumps[LUMP_NODES], &GameLump, 0);
		LoadNodes(&GameLump);
		FreeLump(&GameLump);
	});

	ProfilableCode("visibility",
	[&]()
	{
		BSP::GameLump GameLump;

		LoadLump(file, &Header.lumps[LUMP_VISIBILITY], &GameLump, 0);
		LoadVisibility(&GameLump);
		FreeLump(&GameLump);
	});

	ProfilableCode("models",
	[&]()
	{
		BSP::GameLump GameLump;

		LoadLump(file, &Header.lumps[LUMP_MODELS], &GameLump, 0);
		LoadSubmodels(&GameLump);
		FreeLump(&GameLump);
	});

	ProfilableCode("entities",
	[&]()
	{
		BSP::GameLump GameLump;

		LoadLump(file, &Header.lumps[LUMP_ENTITIES], &GameLump, 0);
		LoadEntityString(&GameLump);
		FreeLump(&GameLump);
	});

	ProfilableCode("sphere lights",
	[&]()
	{
		BSP::GameLump GameLump;

		LoadLump(file, &Header.lumps[LUMP_SPHERELIGHTS], &GameLump, 0);
		LoadSphereLights(&GameLump);
		FreeLump(&GameLump);
	});

	ProfilableCode("terrains",
	[&]()
	{
		BSP::GameLump GameLump;

		LoadLump(file, &Header.lumps[LUMP_TERRAIN], &GameLump, 0);
		LoadTerrain(&GameLump);
		FreeLump(&GameLump);
	});

	ProfilableCode("terrain indexes",
	[&]()
	{
		BSP::GameLump GameLump;

		LoadLump(file, &Header.lumps[LUMP_TERRAININDEXES], &GameLump, 0);
		LoadTerrainIndexes(&GameLump);
		FreeLump(&GameLump);
	});

	if (Header.version > BSP_BETA_VERSION)
	{
		ProfilableCode("static models",
		[&]()
		{
			BSP::GameLump GameLump;

			LoadLump(file, &Header.lumps[LUMP_STATICMODELDEF], &GameLump, 0);
			LoadStaticModelDefs(&GameLump);
			FreeLump(&GameLump);
		});
	}

	ProfilableCode("generation of terrain surfaces",
	[&]()
	{
		CreateTerrainSurfaces();
	});

	ProfilableCode("generation of terrain collision",
	[&]()
	{
		TR_PrepareGerrainCollide();
	});
	
	ProfilableCode("brush mapping",
	[&]()
	{
		MapBrushes();
	});

	ProfilableCode("creation of entities",
	[&]()
	{
		CreateEntities();
	});

	return true;
}

/*
void BSP::PreAllocateLevelData(const File_Header *Header)
{
	size_t memsize;
	memsize = Header->lumps[LUMP_SHADERS].FileLength / sizeof(BSP::File_Shader) * sizeof BSP::Shader;
	memsize += Header->lumps[LUMP_PLANES].FileLength / sizeof(BSP::File_Plane) * sizeof BSP::Plane;
	memsize += Header->lumps[LUMP_SURFACES].FileLength / sizeof(BSP::File_Surface) * sizeof BSP::Surface;
	memsize += Header->lumps[LUMP_DRAWVERTS].FileLength / sizeof(BSP::File_Vertice) * sizeof BSP::Vertice;
	memsize += Header->lumps[LUMP_DRAWINDEXES].FileLength;
	memsize += Header->lumps[LUMP_SIDEEQUATIONS].FileLength / sizeof(BSP::File_SideEquation) * sizeof BSP::SideEquation;
	memsize += Header->lumps[LUMP_BRUSHSIDES].FileLength / sizeof(BSP::File_BrushSide) * sizeof BSP::BrushSide;
	memsize += Header->lumps[LUMP_BRUSHES].FileLength / sizeof(BSP::File_Brush) * sizeof BSP::Brush;
	memsize += Header->lumps[LUMP_MODELS].FileLength / sizeof(BSP::File_BrushModel) * sizeof BSP::Model;
	memsize += Header->lumps[LUMP_ENTITIES].FileLength;
	memsize += Header->lumps[LUMP_SPHERELIGHTS].FileLength / sizeof(BSP::File_SphereLight) * sizeof BSP::SphereLight;
	memsize += Header->lumps[LUMP_TERRAIN].FileLength / sizeof(BSP::File_TerrainPatch) * sizeof BSP::TerrainPatch;
	memsize += Header->lumps[LUMP_STATICMODELDEF].FileLength / sizeof(BSP::File_StaticModel) * sizeof BSP::StaticModel;
	memdata = new uint8_t[memsize];
}
*/

BSP::Plane::PlaneType BSP::PlaneTypeForNormal(const Vector& normal)
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

const BSP::Shader *BSP::GetShader(size_t shaderNum) const
{
	if (shaderNum < shaders.size())
	{
		return &shaders.at(shaderNum);
	}
	else
	{
		return nullptr;
	}
}

size_t BSP::GetNumLightmaps() const
{
	return lightmaps.size();
}

const BSP::Lightmap *BSP::GetLightmap(size_t lightmapNum) const
{
	if (lightmapNum < lightmaps.size())
	{
		return &lightmaps.at(lightmapNum);
	}
	else
	{
		return nullptr;
	}
}

size_t BSP::GetNumSurfaces() const
{
	return surfaces.size();
}

const BSP::Surface *BSP::GetSurface(size_t surfaceNum)
{
	if (surfaceNum < surfaces.size())
	{
		return &surfaces.at(surfaceNum);
	}
	else
	{
		return nullptr;
	}
}

size_t BSP::GetNumPlanes() const
{
	return planes.size();
}

const BSP::Plane *BSP::GetPlane(size_t planeNum)
{
	if (planeNum < planes.size())
	{
		return &planes.at(planeNum);
	}
	else
	{
		return nullptr;
	}
}

size_t BSP::GetNumSideEquations() const
{
	return sideEquations.size();
}

const BSP::SideEquation *BSP::GetSideEquation(size_t equationNum)
{
	if (equationNum < sideEquations.size())
	{
		return &sideEquations.at(equationNum);
	}
	else
	{
		return nullptr;
	}
}

size_t BSP::GetNumBrushSides() const
{
	return brushSides.size();
}

const BSP::BrushSide *BSP::GetBrushSide(size_t brushSideNum)
{
	if (brushSideNum < brushSides.size())
	{
		return &brushSides.at(brushSideNum);
	}
	else
	{
		return nullptr;
	}
}

size_t BSP::GetNumBrushes() const
{
	return brushes.size();
}

const BSP::Brush *BSP::GetBrush(size_t brushNum) const
{
	if (brushNum < brushes.size())
	{
		return &brushes.at(brushNum);
	}
	else
	{
		return nullptr;
	}
}

const BSP::Leaf* BSP::GetLeaf(size_t leafNum) const
{
	if (leafNum < leafs.size())
	{
		return &leafs.at(leafNum);
	}
	else
	{
		return nullptr;
	}
}

size_t BSP::GetNumLeafs() const
{
	return leafs.size();
}

const BSP::Node* BSP::GetNode(size_t nodeNum) const
{
	if (nodeNum < nodes.size())
	{
		return &nodes.at(nodeNum);
	}
	else
	{
		return nullptr;
	}
}

size_t BSP::GetNumNodes() const
{
	return nodes.size();
}

uintptr_t BSP::GetLeafBrush(size_t leafBrushNum) const
{
	if (leafBrushNum < leafBrushes.size())
	{
		return leafBrushes.at(leafBrushNum);
	}
	else
	{
		return 0;
	}
}

size_t BSP::GetNumLeafBrushes() const
{
	return leafBrushes.size();
}

uintptr_t BSP::GetLeafSurface(size_t leafSurfNum) const
{
	if (leafSurfNum < leafSurfaces.size())
	{
		return leafSurfaces.at(leafSurfNum);
	}
	else
	{
		return 0;
	}
}

size_t BSP::GetNumLeafSurfaces() const
{
	return leafSurfaces.size();
}

size_t BSP::GetNumSubmodels() const
{
	return brushModels.size();
}

const BSP::Model *BSP::GetSubmodel(size_t submodelNum) const
{
	if (submodelNum < brushModels.size())
	{
		return &brushModels.at(submodelNum);
	}
	else
	{
		return nullptr;
	}
}

const BSP::Model *BSP::GetSubmodel(const str& submodelName) const
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

const BSP::SphereLight *BSP::GetLight(size_t lightNum) const
{
	if (lightNum < lights.size())
	{
		return &lights.at(lightNum);
	}
	else
	{
		return nullptr;
	}
}

size_t BSP::GetNumStaticModels() const
{
	return staticModels.size();
}

const BSP::StaticModel *BSP::GetStaticModel(size_t staticModelNum) const
{
	if (staticModelNum < staticModels.size())
	{
		return &staticModels.at(staticModelNum);
	}
	else
	{
		return nullptr;
	}
}

size_t BSP::GetNumTerrainPatches() const
{
	return terrainPatches.size();
}

const BSP::TerrainPatch *BSP::GetTerrainPatch(size_t terrainPatchNum) const
{
	if (terrainPatchNum < terrainPatches.size())
	{
		return &terrainPatches.at(terrainPatchNum);
	}
	else
	{
		return nullptr;
	}
}

size_t BSP::GetNumTerrainSurfaces() const
{
	return terrainSurfaces.size();
}

const BSP::Surface *BSP::GetTerrainSurface(size_t terrainSurfaceNum) const
{
	if (terrainSurfaceNum < terrainSurfaces.size())
	{
		return &terrainSurfaces.at(terrainSurfaceNum);
	}
	else
	{
		return nullptr;
	}
}

size_t BSP::GetNumEntities() const
{
	return entities.size();
}

const LevelEntity* BSP::GetEntity(size_t entityNum) const
{
	if (entityNum < entities.size())
	{
		return entities.at(entityNum);
	}
	else
	{
		return nullptr;
	}
}

const LevelEntity* BSP::GetEntity(const str& targetName) const
{
	const Container<LevelEntity *>* ents = GetEntities(targetName);
	if (ents && ents->size())
	{
		return ents->at(0);
	}
	else
	{
		return nullptr;
	}
}

const Container<LevelEntity *>* BSP::GetEntities(const str& targetName) const
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
	const Container<LevelEntity*>* ents = targetList.findKeyValue(targetName);
	if (ents) {
		return ents;
	}
	else {
		return nullptr;
	}
}

size_t BSP::GetNumSurfacesGroup() const
{
	return surfacesGroups.size();
}

const BSP::SurfacesGroup *BSP::GetSurfacesGroup(size_t surfsGroupNum) const
{
	if (surfsGroupNum < surfacesGroups.size())
	{
		return surfacesGroups.at(surfsGroupNum);
	}
	else
	{
		return nullptr;
	}
}

void BSP::LoadShaders(const BSP::GameLump* GameLump)
{
	if (GameLump->Length % sizeof(File_Shader))
	{
		return;
	}

	size_t count = GameLump->Length / sizeof(File_Shader);
	if (count)
	{
		shaders.resize(count);

		const File_Shader* in = (File_Shader *)GameLump->Buffer;
		Shader* out = shaders.data();

		for (size_t i = 0; i < count; i++, in++, out++)
		{
			out->surfaceFlags = in->surfaceFlags;
			out->contentFlags = in->contentFlags;
			out->shaderName = in->shader;
			out->subdivisions = in->subdivisions;
			out->shader = GetAssetManager()->GetManager<ShaderManager>()->GetShader(in->shader);
		}
	}
}

void BSP::LoadLightmaps(const GameLump* GameLump)
{
	if (!GameLump->Length)
	{
		return;
	}

	const size_t numLightmaps = GameLump->Length / lightmapMemSize;
	lightmaps.resize(numLightmaps);

	const uint8_t* buf = (uint8_t*)GameLump->Buffer;
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

void BSP::ParseMesh(const File_Surface* InSurface, const File_Vertice* InVertices, Surface* Out)
{
	Out->shader = GetShader(InSurface->shaderNum);

	int32_t Width = InSurface->patchWidth;
	int32_t Height = InSurface->patchHeight;

	if (Width < 0 || Width > MAX_PATCH_SIZE || Height < 0 || Height > MAX_PATCH_SIZE)
	{
		return;
	}

	int32_t NumPoints = Width * Height;
	Vertice Points[MAX_PATCH_SIZE * MAX_PATCH_SIZE];

	InVertices += InSurface->firstVert;
	for (int32_t i = 0; i < NumPoints; i++)
	{
		for (int32_t j = 0; j < 3; j++)
		{
			Points[i].xyz[j] = InVertices[i].xyz[j];
			Points[i].normal[j] = InVertices[i].normal[j];
		}

		AddPointToBounds(Points[i].xyz, Out->cullInfo.bounds[0], Out->cullInfo.bounds[1]);

		for (int32_t j = 0; j < 2; j++)
		{
			Points[i].st[j] = InVertices[i].st[j];
			Points[i].lightmap[j] = InVertices[i].lightmap[j];
		}

		Points[i].color[0] = InVertices[i].color[0];
		Points[i].color[1] = InVertices[i].color[1];
		Points[i].color[2] = InVertices[i].color[2];
		Points[i].color[3] = InVertices[i].color[3];
		//color[0] = InVertices[i].color[0];
		//color[1] = InVertices[i].color[1];
		//color[2] = InVertices[i].color[2];
		//color[3] = InVertices[i].color[3] / 255.0f;

		//ColorShiftLightingFloats(color, Points[i].color, 1.0f / 255.0f);
	}

	SubdividePatchToGrid(Width, Height, Points, Out);
	Out->bIsPatch = true;
	Out->pc = GeneratePatchCollide(Width, Height, Points, (float)Out->shader->subdivisions);
}

void BSP::ParseFace(const File_Surface* InSurface, const File_Vertice* InVertices, const int32_t* InIndices, Surface* Out)
{
	uint32_t i, j;

	size_t numVerts = InSurface->numVerts;
	size_t numIndexes = InSurface->numIndexes;

	Out->shader = GetShader(InSurface->shaderNum);

	Out->indexes.resize(numIndexes);
	Out->vertices.resize(numVerts);

	// copy vertexes
	Out->cullInfo.type = Out->cullInfo.CULLINFO_PLANE | Out->cullInfo.CULLINFO_BOX;
	ClearBounds(Out->cullInfo.bounds[0], Out->cullInfo.bounds[1]);
	InVertices += InSurface->firstVert;
	Vertice *OutVertices = Out->vertices.data();
	for (i = 0; i < numVerts; i++)
	{
		for (j = 0; j < 3; j++)
		{
			OutVertices[i].xyz[j] = InVertices[i].xyz[j];
			OutVertices[i].normal[j] = InVertices[i].normal[j];
		}

		AddPointToBounds(OutVertices[i].xyz, Out->cullInfo.bounds[0], Out->cullInfo.bounds[1]);

		for (j = 0; j < 2; j++)
		{
			OutVertices[i].st[j] = InVertices[i].st[j];
			OutVertices[i].lightmap[j] = InVertices[i].lightmap[j];
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
	InIndices += InSurface->firstIndex;
	size_t* tri;
	for (i = 0, tri = Out->indexes.data(); i < numIndexes; i += 3, tri += 3)
	{
		for (j = 0; j < 3; j++)
		{
			tri[j] = InIndices[i + j];

			if (tri[j] >= numVerts)
			{
				return;
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
		Out->indexes.resize(Out->indexes.size() - badTriangles * 3);
	}

	for (i = 0; i < 3; i++)
	{
		Out->cullInfo.plane.normal[i] = InSurface->lightmapVecs[2][i];
	}

	Out->cullInfo.plane.distance = (float)Vector::Dot(Out->vertices[0].xyz, Out->cullInfo.plane.normal);
	Out->cullInfo.plane.type = PlaneTypeForNormal(Out->cullInfo.plane.normal);
}

void BSP::ParseTriSurf(const File_Surface* InSurface, const File_Vertice* InVertices, const int32_t* InIndices, Surface* Out)
{
	uint32_t i, j;
	size_t numVerts = InSurface->numVerts;
	size_t numIndexes = InSurface->numIndexes;

	Out->shader = GetShader(InSurface->shaderNum);

	Out->indexes.resize(numIndexes);
	Out->vertices.resize(numVerts);

	// copy vertexes
	Out->cullInfo.type = Out->cullInfo.CULLINFO_BOX;
	ClearBounds(Out->cullInfo.bounds[0], Out->cullInfo.bounds[1]);
	InVertices += InSurface->firstVert;
	Vertice *OutVertices = Out->vertices.data();
	for (i = 0; i < numVerts; i++)
	{
		for (j = 0; j < 3; j++)
		{
			OutVertices[i].xyz[j] = InVertices[i].xyz[j];
			OutVertices[i].normal[j] = InVertices[i].normal[j];
		}

		AddPointToBounds(Out->vertices[i].xyz, Out->cullInfo.bounds[0], Out->cullInfo.bounds[1]);

		for (j = 0; j < 2; j++)
		{
			OutVertices[i].st[j] = InVertices[i].st[j];
			OutVertices[i].lightmap[j] = InVertices[i].lightmap[j];
		}

		Out->vertices[i].color[0] = InVertices[i].color[0];
		Out->vertices[i].color[1] = InVertices[i].color[1];
		Out->vertices[i].color[2] = InVertices[i].color[2];
		Out->vertices[i].color[3] = InVertices[i].color[3];
		//color[0] = InVertices[i].color[0];
		//color[1] = InVertices[i].color[1];
		//color[2] = InVertices[i].color[2];
		//color[3] = InVertices[i].color[3] / 255.0f;

		//ColorShiftLightingFloats(color, Out->vertices[i].color, 1.0f / 255.0f);
	}

	// copy triangles
	size_t badTriangles = 0;
	size_t* tri;
	InIndices += InSurface->firstIndex;
	for (i = 0, tri = Out->indexes.data(); i < numIndexes; i += 3, tri += 3)
	{
		for (j = 0; j < 3; j++)
		{
			tri[j] = InIndices[i + j];

			if (tri[j] >= numVerts)
			{
				return;
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
		Out->indexes.resize(Out->indexes.size() - badTriangles * 3);
	}
}

void BSP::LoadSurfaces(const GameLump* surfaces, const GameLump* vertices, const GameLump* Indices)
{
	enum SurfaceType
	{
		MST_BAD,
		MST_PLANAR,
		MST_PATCH,
		MST_TRIANGLE_SOUP,
		MST_FLARE
	};

	if (surfaces->Length % sizeof(File_Surface))
	{
		return;
	}

	if (vertices->Length % sizeof(File_Vertice))
	{
		return;
	}

	if (Indices->Length % sizeof(int32_t))
	{
		return;
	}

	size_t Count = surfaces->Length / sizeof(File_Surface);
	if (Count)
	{
		this->surfaces.resize(Count);

		const File_Surface* In = (File_Surface*)surfaces->Buffer;
		Surface* Out = this->surfaces.data();

		const File_Vertice* InVerts = (File_Vertice*)vertices->Buffer;
		int32_t* InIndexes = (int32_t*)Indices->Buffer;

		for (size_t i = 0; i < Count; i++, In++, Out++)
		{
			Out->lightmapNum = In->lightmapNum;
			Out->lightmapX = In->lightmapX;
			Out->lightmapY = In->lightmapY;
			Out->lightmapWidth = In->lightmapWidth;
			Out->lightmapHeight = In->lightmapHeight;
			Out->lightmapOrigin = In->lightmapOrigin;
			Out->lightmapVecs[0] = In->lightmapVecs[0];
			Out->lightmapVecs[1] = In->lightmapVecs[1];
			Out->lightmapVecs[2] = In->lightmapVecs[2];

			switch (In->surfaceType)
			{
			case MST_PATCH:
				ParseMesh(In, InVerts, Out);
				break;
			case MST_TRIANGLE_SOUP:
				ParseTriSurf(In, InVerts, InIndexes, Out);
				break;
			case MST_PLANAR:
				ParseFace(In, InVerts, InIndexes, Out);
				break;
			}

			Out->CalculateCentroid();
		}
	}
}

void BSP::LoadPlanes(const GameLump* GameLump)
{
	if (GameLump->Length % sizeof(File_Plane))
	{
		return;
	}

	size_t Count = GameLump->Length / sizeof(File_Plane);
	if (Count)
	{
		planes.resize(Count);

		const File_Plane* In = (File_Plane*)GameLump->Buffer;
		Plane* Out = planes.data();

		for (size_t i = 0; i < Count; i++, In++, Out++)
		{
			uint8_t Bits = 0;
			for (int32_t j = 0; j < 3; j++)
			{
				Out->normal[j] = In->normal[j];
				if (Out->normal[j] < 0)
				{
					Bits |= 1 << j;
				}
			}

			Out->distance = In->dist;
			Out->type = PlaneTypeForNormal(Out->normal);
			Out->signBits = Bits;
		}
	}
}

void BSP::LoadSideEquations(const GameLump* GameLump)
{
	if (GameLump->Length % sizeof(File_SideEquation))
	{
		return;
	}

	size_t Count = GameLump->Length / sizeof(File_SideEquation);
	if (Count)
	{
		sideEquations.resize(Count);

		const File_SideEquation* In = (File_SideEquation*)GameLump->Buffer;
		SideEquation* Out = sideEquations.data();

		for (size_t i = 0; i < Count; i++, In++, Out++)
		{
			for (int32_t j = 0; j < 4; j++)
			{
				Out->sEq[j] = In->seq[j];
				Out->tEq[j] = In->teq[j];
			}
		}
	}
}

void BSP::LoadBrushSides(const GameLump* GameLump)
{
	if (GameLump->Length % sizeof(File_BrushSide))
	{
		return;
	}

	size_t Count = GameLump->Length / sizeof(File_BrushSide);
	if (Count)
	{
		brushSides.resize(Count);

		const File_BrushSide* In = (File_BrushSide*)GameLump->Buffer;
		BrushSide* Out = brushSides.data();

		for (size_t i = 0; i < Count; i++, In++, Out++)
		{
			int32_t Num = In->planeNum;
			Out->plane = &planes[Num];
			Out->shader = GetShader(In->shaderNum);
			Out->surfaceFlags = Out->shader->surfaceFlags;

			Num = In->equationNum;
			if (Num)
			{
				Out->Eq = &sideEquations[Num];
			}
			else
			{
				Out->Eq = NULL;
			}
		}
	}
}

void BSP::LoadBrushes(const GameLump* GameLump)
{
	if (GameLump->Length % sizeof(File_Brush))
	{
		return;
	}

	size_t Count = GameLump->Length / sizeof(File_Brush);
	if (Count)
	{
		brushes.resize(Count);

		const File_Brush* In = (File_Brush*)GameLump->Buffer;
		Brush* Out = brushes.data();

		for (size_t i = 0; i < Count; i++, In++, Out++)
		{
			Out->sides = &brushSides[In->firstSide];
			Out->numsides = In->numSides;

			Out->shader = GetShader(In->shaderNum);

			Out->contents = Out->shader->contentFlags;
			Out->parent = NULL;

			BoundBrush(Out);
		}
	}
}

void BSP::LoadLeafs(const GameLump* GameLump)
{
	numClusters = 0;
	numAreas = 0;

	if (GameLump->Length % sizeof(File_Leaf))
	{
		return;
	}

	size_t Count = GameLump->Length / sizeof(File_Leaf);
	if (Count)
	{
		leafs.resize(Count);

		const File_Leaf* in = (File_Leaf*)GameLump->Buffer;
		Leaf* out = leafs.data();

		for (size_t i = 0; i < Count; ++i, ++in, ++out)
		{
			out->cluster = in->cluster;
			out->area = in->area;
			out->firstLeafBrush = in->firstLeafBrush;
			out->numLeafBrushes = in->numLeafBrushes;
			out->firstLeafSurface =in->firstLeafSurface;
			out->numLeafSurfaces = in->numLeafSurfaces;
			out->firstLeafTerrain = in->firstTerraPatch;
			out->numLeafTerrains = in->numTerraPatches;

			if (out->cluster >= numClusters) numClusters = out->cluster + 1;
			if (out->area >= numAreas) numAreas = out->area + 1;
		}
	}

	areas.resize(numAreas);
	areaPortals.resize(numAreas * numAreas);
}

void BSP::LoadLeafsOld(const GameLump* GameLump)
{
	numClusters = 0;
	numAreas = 0;

	if (GameLump->Length % sizeof(File_Leaf))
	{
		return;
	}

	size_t Count = GameLump->Length / sizeof(File_Leaf_Ver17);
	if (Count)
	{
		leafs.resize(Count);

		const File_Leaf_Ver17* in = (File_Leaf_Ver17*)GameLump->Buffer;
		Leaf* out = leafs.data();

		for (size_t i = 0; i < Count; ++i, ++in, ++out)
		{
			out->cluster = in->cluster;
			out->area = in->area;
			out->firstLeafBrush = in->firstLeafBrush;
			out->numLeafBrushes = in->numLeafBrushes;
			out->firstLeafSurface = in->firstLeafSurface;
			out->numLeafSurfaces = in->numLeafSurfaces;

			if (out->cluster >= numClusters) numClusters = out->cluster + 1;
			if (out->area >= numAreas) numAreas = out->area + 1;
		}
	}

	areas.resize(numAreas);
	areaPortals.resize(numAreas * numAreas);
}

void BSP::LoadLeafsBrushes(const GameLump* GameLump)
{
	if (GameLump->Length % sizeof(uint32_t))
	{
		return;
	}

	size_t count = GameLump->Length / sizeof(uint32_t);
	if (count)
	{
		leafBrushes.resize(count);

		uint32_t* in = (uint32_t*)GameLump->Buffer;

		for (size_t i = 0; i < count; ++i) {
			leafBrushes[i] = in[i];
		}
	}
}

void BSP::LoadLeafSurfaces(const GameLump* GameLump)
{
	if (GameLump->Length % sizeof(uint32_t))
	{
		return;
	}

	size_t count = GameLump->Length / sizeof(uint32_t);
	if (count)
	{
		leafSurfaces.resize(count);

		uint32_t* in = (uint32_t*)GameLump->Buffer;

		for (size_t i = 0; i < count; ++i) {
			leafSurfaces[i] = in[i];
		}
	}
}

void BSP::LoadNodes(const GameLump* GameLump)
{
	if (GameLump->Length % sizeof(File_Node))
	{
		return;
	}

	size_t count = GameLump->Length / sizeof(File_Node);
	if (count)
	{
		nodes.resize(count);

		File_Node* in = (File_Node*)GameLump->Buffer;
		Node* out = nodes.data();

		for (size_t i = 0; i < count; ++i, ++in, ++out)
		{
			out->plane = &planes[in->planeNum];

			for (int j = 0; j < 2; ++j) {
				out->children[j] = in->children[j];
			}
		}
	}
}

#define	VIS_HEADER	8

void BSP::LoadVisibility(const GameLump* GameLump)
{
	size_t count = GameLump->Length;
	if (!count)
	{
		clusterBytes = (numClusters + 31) & ~31;
		visibility.resize(clusterBytes);
		memset(visibility.data(), 255, clusterBytes);
		return;
	}

	visibility.resize(count);
	numClusters = ((uint32_t*)GameLump->Buffer)[0];
	clusterBytes = ((uint32_t*)GameLump->Buffer)[1];
	memcpy(visibility.data(), (uint8_t*)GameLump->Buffer + VIS_HEADER, count - VIS_HEADER);
}

void BSP::LoadSubmodels(const GameLump* GameLump)
{
	if (GameLump->Length % sizeof(File_BrushModel))
	{
		return;
	}

	size_t Count = GameLump->Length / sizeof(File_BrushModel);
	if (Count)
	{
		brushModels.resize(Count);

		const File_BrushModel* In = (File_BrushModel*)GameLump->Buffer;
		Model* Out = brushModels.data();

		for (size_t i = 0; i < Count; i++, In++, Out++)
		{
			Out->bounds[0] = In->mins;
			Out->bounds[1] = In->maxs;
			Out->numSurfaces = In->numSurfaces;
			if (Out->numSurfaces)
			{
				Out->surface = &surfaces[In->firstSurface];
			}
			else
			{
				Out->surface = nullptr;
			}
		}
	}
}

void BSP::LoadEntityString(const GameLump* GameLump)
{
	entityString = new char[GameLump->Length];
	entityStringLength = GameLump->Length;
	memcpy(entityString, GameLump->Buffer, entityStringLength);
}

void BSP::LoadSphereLights(const GameLump* GameLump)
{
	if (GameLump->Length % sizeof(File_SphereLight))
	{
		return;
	}

	size_t NumLights = GameLump->Length / sizeof(File_SphereLight);
	if (NumLights)
	{
		lights.resize(NumLights);

		const File_SphereLight* In = (File_SphereLight *)GameLump->Buffer;
		SphereLight* Out = lights.data();

		for (size_t i = 0; i < NumLights; In++, Out++, i++)
		{
			Out->origin = In->origin;
			Out->color = In->color;
			Out->spotDirection = In->spot_dir;

			Out->spotRadiusByDistance = In->spot_radiusbydistance;
			Out->intensity = In->intensity;
			Out->bSpotLight = In->spot_light;
			Out->bNeedsTrace = In->needs_trace;
		}
	}
}

void BSP::LoadStaticModelDefs(const GameLump* GameLump)
{
	if (!GameLump->Length)
	{
		return;
	}

	if (GameLump->Length % sizeof(File_StaticModel))
	{
		return;
	}

	size_t NumStaticModels = GameLump->Length / sizeof(File_StaticModel);
	if (NumStaticModels > 0)
	{
		staticModels.resize(NumStaticModels);

		const File_StaticModel* In = (File_StaticModel *)GameLump->Buffer;
		StaticModel* Out = staticModels.data();

		for (size_t i = 0; i < NumStaticModels; In++, Out++, i++)
		{
			Out->visCount = 0;
			Out->angles = In->angles;
			Out->origin = In->origin;
			Out->scale = In->scale;
			Out->firstVertexData = In->firstVertexData;
			Out->numVertexData = In->numVertexData;
			Out->modelName = CanonicalModelName(In->model);
		}
	}
}

#define LIGHTMAP_SIZE 128
#define TERRAIN_LIGHTMAP_LENGTH (16.f / LIGHTMAP_SIZE)

void BSP::UnpackTerraPatch(const File_TerrainPatch* Packed, TerrainPatch* Unpacked) const
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
	Unpacked->z0 = Packed->iBaseHeight;
	Unpacked->shader = GetShader(Packed->iShader);
	Unpacked->north = Packed->iNorth;
	Unpacked->east = Packed->iEast;
	Unpacked->south = Packed->iSouth;
	Unpacked->west = Packed->iWest;

	for (i = 0; i < 63; i++)
	{
		flags.v = Packed->varTree[0][i].flags;
		flags.b[1] &= 7;
		Unpacked->varTree[0][i].variance = flags.v;
		Unpacked->varTree[0][i].s.flags = Packed->varTree[0][i].flags >> 12;

		flags.v = Packed->varTree[1][i].flags;
		flags.b[1] &= 7;
		Unpacked->varTree[1][i].variance = flags.v;
		Unpacked->varTree[1][i].s.flags = Packed->varTree[1][i].flags >> 12;
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

void BSP::LoadTerrain(const GameLump* GameLump)
{
	if (!GameLump->Length)
	{
		return;
	}

	if (GameLump->Length % sizeof(File_TerrainPatch))
	{
		return;
	}

	size_t Count = GameLump->Length / sizeof(File_TerrainPatch);
	if (Count > 0)
	{
		terrainPatches.resize(Count);

		File_TerrainPatch* In = (File_TerrainPatch *)GameLump->Buffer;
		TerrainPatch* Out = &*terrainPatches.begin();

		for (size_t i = 0; i < Count; In++, Out++, i++)
		{
			UnpackTerraPatch(In, Out);
		}
	}
}


void BSP::LoadTerrainIndexes(const GameLump* GameLump)
{
	if (!GameLump->Length) {
		return;
	}

	if (GameLump->Length % sizeof(uint16_t)) {
		return;
	}

	size_t count = GameLump->Length / sizeof(uint16_t);
	if (count > 0)
	{
		leafTerrains.resize(count);

		uint16_t* in = (uint16_t*)GameLump->Buffer;

		for (size_t i = 0; i < count; ++i) {
			leafTerrains[i] = &terrainPatches[in[i]];
		}
	}
}

void MOHPC::BSP::FloodArea(size_t areaNum, uint32_t floodNum, uint32_t& floodValid)
{
	Area* area = &areas[areaNum];

	if(area->floodValid == floodValid)
	{
		if (area->floodNum == floodNum) {
			return;
		}

		// FIXME: Should throw exception "FloodArea_r: reflooded"
		return;
	}

	area->floodNum = floodNum;
	area->floodValid = floodValid;
	uintptr_t* con = &areaPortals[areaNum * numAreas];
	for (size_t i = 0; i < numAreas; ++i)
	{
		if (con[i] > 0) {
			FloodArea(i, floodNum, floodValid);
		}
	}
}

void BSP::FloodAreaConnections()
{
	uint32_t floodValid = 1;
	uint32_t floodNum = 0;

	for (size_t i = 0; i < areas.size(); ++i)
	{
		Area* area = &areas[i];
		if (area->floodValid == floodValid) {
			continue;
		}

		++floodNum;
		FloodArea(i, floodNum, floodValid);
	}
}

void BSP::BoundBrush(Brush* Brush)
{
	Brush->bounds[0][0] = -Brush->sides[0].plane->distance;
	Brush->bounds[1][0] = Brush->sides[1].plane->distance;

	Brush->bounds[0][1] = -Brush->sides[2].plane->distance;
	Brush->bounds[1][1] = Brush->sides[3].plane->distance;

	Brush->bounds[0][2] = -Brush->sides[4].plane->distance;
	Brush->bounds[1][2] = Brush->sides[5].plane->distance;
}

void BSP::ColorShiftLightingFloats(float in[4], float out[4], float scale)
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

void BSP::ColorShiftLightingFloats3(vec3_t in, vec3_t out, float scale)
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

int32_t BSP::LoadLump(FilePtr file, BSP::File_Lump* Lump, BSP::GameLump* GameLump, size_t Size)
{
	GameLump->Length = Lump->FileLength;

	if (Lump->FileLength)
	{
		GameLump->Buffer = new char[Lump->FileLength];

		std::istream* Stream = file->GetStream();

		Stream->seekg(Lump->FileOffset, Stream->beg);
		Stream->read((char*)GameLump->Buffer, Lump->FileLength);

		HashUpdate((uint8_t*)GameLump->Buffer, Lump->FileLength);

		if (Size)
		{
			return (int32_t)(Lump->FileLength / Size);
		}
	}
	else
	{
		GameLump->Buffer = nullptr;
	}

	return 0;
}

void BSP::FreeLump(BSP::GameLump* GameLump)
{
	if (GameLump->Buffer)
	{
		delete[] (char*)GameLump->Buffer;
	}
}

void BSP::CreateEntities()
{
	Script script;

	script.Parse(entityString, entityStringLength);

	while (script.TokenAvailable(true))
	{
		const char *token = script.GetToken(false);
		if (!stricmp(token, "{"))
		{
			PropertyMap propertiesMap;

			while (script.TokenAvailable(true))
			{
				token = script.GetToken(true);
				if (!stricmp(token, "}"))
				{
					break;
				}

				str key = token;
				str value;

				key.tolower();
				//std::transform(key.begin(), key.end(), key.begin(), ::tolower);

				/*
				if (!stricmp(token, "targetname"))
				{
					token = script.GetToken(false);
					propertiesMap.insert_or_assign("targetname", token);
				}
				else if (!stricmp(token, "target"))
				{
					token = script.GetToken(false);
					propertiesMap.insert_or_assign("target", token);
				}
				else if (!stricmp(token, "classname"))
				{
					token = script.GetToken(false);
					propertiesMap.insert_or_assign("classname", token);
				}
				else if (!stricmp(token, "model"))
				{
					token = script.GetToken(false);
					propertiesMap.insert_or_assign("model", CanonicalModelName(token));
				}
				else if (!stricmp(token, "origin"))
				{
					token = script.GetString(false);
					propertiesMap.insert_or_assign("origin", token);
				}
				else if (!stricmp(token, "angles"))
				{
					token = script.GetString(false);
					propertiesMap.insert_or_assign("angles", token);
				}
				else if (!stricmp(token, "angle"))
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
						value += script.GetToken(false);
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

			// Insert the entity to the list of named entities
			if (*ent->GetTargetName()) {
				//targetList[ent->GetTargetName()].push_back(ent);
				targetList.addKeyValue(ent->GetTargetName()).AddObject(ent);
			}

			entities.push_back(ent);
		}
	}

	if (entityString)
	{
		delete[] entityString;
		entityString = nullptr;
		entityStringLength = 0;
	}
}

void BSP::MapBrushes()
{
	size_t numParentedBrushes = 0;
	const size_t numBrushes = brushes.size();

	Brush* brushesList = brushes.data();

	// Connects brushes by finding touching brushes that share at least one same shader
	for (size_t b = 0; b < numBrushes; b++)
	{
		Brush* brush = &brushesList[b];
		brush->name = "brush" + str(b); // std::to_string(b);
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
					BrushSide* brushside = &brush->sides[bs];
					if (brushside->surfaceFlags & SURF_NODRAW)
					{
						continue;
					}

					for (size_t bs2 = 0; bs2 < brush2->numsides; bs2++)
					{
						BrushSide* brushside2 = &brush2->sides[bs2];
						if (brushside2->surfaceFlags & SURF_NODRAW)
						{
							continue;
						}

						if (!stricmp(brushside2->shader->shaderName.c_str(), brushside->shader->shaderName.c_str()))
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
	Model* worldModel = &brushModels[0];

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
		const Vector& mins = brush->bounds[0];
		const Vector& maxs = brush->bounds[1];

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
						if (!stricmp(side->shader->shaderName.c_str(), surf->shader->shaderName.c_str()))
						{
							brush->surfaces.push_back(surf);

							SurfacesGroup *sg = nullptr;
							if (it == brushToSurfaces.end())
							{
								sg = new SurfacesGroup;
								sg->name = "surfacesgroup" + str(numSurfacesGroups); // std::to_string(numSurfacesGroups);
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
		Container<Patch> patchList;
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
					rootPatch->surfaceGroup->name = "meshpatch_grouped" + str(numGroupedPatches++); // std::to_string(numGroupedPatches++);
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
			sg->name = "surfacesgroup_unmapped" + str(numUnmappedSurfaces); // std::to_string(numUnmappedSurfaces);
			sg->surfaces.push_back(surf);
			surfacesGroups.push_back(sg);

			numUnmappedSurfaces++;
		}
	}

	// Calculate the origin for each group of surfaces
	for (size_t i = 0; i < surfacesGroups.size(); i++)
	{
		SurfacesGroup *sg = surfacesGroups[i];

		Vector avg(0, 0, 0);
		size_t numVertices = 0;

		for (size_t k = 0; k < sg->surfaces.size(); k++)
		{
			const Surface* surf = sg->surfaces[k];

			for (size_t v = 0; v < surf->vertices.size(); v++)
			{
				const Vertice* vert = &surf->vertices[v];

				avg += vert->xyz;

				AddPointToBounds(surf->vertices[v].xyz, sg->bounds[0], sg->bounds[1]);
			}

			numVertices += surf->vertices.size();
		}

		sg->origin = avg / (float)numVertices;
	}

	surfacesGroups.shrink_to_fit();
	delete[] mappedSurfaces;
}

uintptr_t BSP::PointLeafNum(const MOHPC::Vector p)
{
	if (!nodes.size()) {
		return 0;
	}

	return PointLeafNum_r(p, 0);
}

uintptr_t BSP::PointLeafNum_r(const MOHPC::Vector p, intptr_t num)
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
