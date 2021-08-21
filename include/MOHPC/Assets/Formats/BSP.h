#pragma once

#include "../Asset.h"
#include "../Managers/ShaderManager.h"
#include "../../Files/File.h"
#include "../../Common/str.h"
#include "../../Common/Vector.h"
#include "../../Utility/SharedPtr.h"

#include "BSP_Terrain.h"

#include <vector>
#include <exception>
#include <cstdint>

namespace MOHPC
{
	struct worknode_t;

	class BSP;
	class File;
	class Shader;
	class CollisionWorld;
	struct patchWork_t;

	namespace BSPFile
	{
		struct GameLump;
		struct flump_t;
		struct fheader_t;
		struct fshader_t;
		struct fsurface_t;
		struct fvertice_t;
		struct fplane_t;
		struct fsideequation_t;
		struct fbrushSide_t;
		struct fbrush_t;
		struct fbmodel_t;
		struct fsphereLight_t;
		struct fstaticModel_t;
		struct fterrainPatch_t;
		struct fnode_t;
		struct fleaf_t;
		struct fleaf_ver17_t;
	}

	namespace BSPData
	{
		static constexpr size_t lightmapSize = 128;
		static constexpr size_t lightmapMemSize = lightmapSize * lightmapSize * sizeof(uint8_t) * 3;
		static constexpr float lightmapTerrainLength = 16.f / lightmapSize;

		struct PatchCollide;

		struct Shader
		{
			str shaderName;
			uint32_t surfaceFlags;
			uint32_t contentFlags;
			uint32_t subdivisions;
			str fenceMaskImage;

			ShaderPtr shader;
		};

		struct Plane
		{
			enum PlaneType
			{
				PLANE_X,
				PLANE_Y,
				PLANE_Z,
				PLANE_NON_AXIAL,
				PLANE_NON_PLANAR
			};

			vec3_t normal;
			float distance;
			PlaneType type;
			uint8_t signBits;
		};

		struct PatchPlane
		{

		public:
			PatchPlane();

		public:
			float plane[4];
			uint8_t signbits;
		};

		struct Facet
		{
			int32_t surfacePlane;
			int32_t numBorders;
			int32_t borderPlanes[4 + 6 + 16];
			bool borderInward[4 + 6 + 16];
			bool borderNoAdjust[4 + 6 + 16];
		};

		struct Vertice
		{

		public:
			Vertice();

		public:
			vec3_t xyz;
			float st[2];
			float lightmap[2];
			vec3_t normal;
			uint8_t color[4];
		};

		class Lightmap
		{
			friend BSP;

		public:
			/** Returns the number of pixels in the lightmap. */
			MOHPC_ASSETS_EXPORTS size_t GetNumPixels() const;

			/** Returns the width of the lightmap. */
			MOHPC_ASSETS_EXPORTS size_t GetWidth() const;

			/** Returns the height of the lightmap. */
			MOHPC_ASSETS_EXPORTS size_t GetHeight() const;

			/** Returns the RGB color of the specified pixel. */
			MOHPC_ASSETS_EXPORTS void GetColor(size_t pixelNum, uint8_t(&out)[3]) const;

		public:
			uint8_t color[lightmapSize * lightmapSize][3];
		};

		class Surface
		{
			friend BSP;

		public:
			Surface();
			~Surface();

			MOHPC_ASSETS_EXPORTS const Shader* GetShader() const;

			MOHPC_ASSETS_EXPORTS size_t GetNumVertices() const;
			MOHPC_ASSETS_EXPORTS const Vertice* GetVertice(size_t index) const;

			MOHPC_ASSETS_EXPORTS size_t GetNumIndexes() const;
			MOHPC_ASSETS_EXPORTS uint32_t GetIndice(size_t index) const;

			MOHPC_ASSETS_EXPORTS uint32_t getWidth() const;
			MOHPC_ASSETS_EXPORTS uint32_t getHeight() const;
			MOHPC_ASSETS_EXPORTS int32_t GetLightmapNum() const;
			MOHPC_ASSETS_EXPORTS int32_t GetLightmapX() const;
			MOHPC_ASSETS_EXPORTS int32_t GetLightmapY() const;
			MOHPC_ASSETS_EXPORTS int32_t GetLightmapWidth() const;
			MOHPC_ASSETS_EXPORTS int32_t GetLightmapHeight() const;
			MOHPC_ASSETS_EXPORTS const_vec3p_t GetLightmapOrigin() const;
			MOHPC_ASSETS_EXPORTS const_vec3p_t GetLightmapVec(int32_t num) const;
			MOHPC_ASSETS_EXPORTS const PatchCollide* GetPatchCollide() const;

			MOHPC_ASSETS_EXPORTS bool IsPatch() const;

		public:
			void CalculateCentroid();

		public:
			const Shader* shader;
			std::vector<Vertice> vertices;
			std::vector<uint32_t> indexes;
			vec3_t centroid;
			bool bIsPatch;
			int32_t lightmapNum;
			int32_t lightmapX;
			int32_t lightmapY;
			int32_t lightmapWidth;
			int32_t lightmapHeight;
			uint32_t width;
			uint32_t height;
			vec3_t lightmapOrigin;
			vec3_t lightmapVecs[3];
			PatchCollide* pc;

			struct
			{
				enum
				{
					CULLINFO_NONE,
					CULLINFO_BOX,
					CULLINFO_SPHERE,
					CULLINFO_PLANE
				};

				int32_t type;
				vec3_t bounds[2];
				vec3_t localOrigin;
				float radius;
				Plane plane;
			} cullInfo;
		};

		struct SideEquation
		{
			float sEq[4];
			float tEq[4];
		};

		struct BrushSide
		{
			const Plane* plane;
			int32_t surfaceFlags;
			const Shader* shader;
			const SideEquation* Eq;
		};

		struct Brush
		{
			const Shader* shader;
			int32_t contents;
			vec3_t bounds[2];
			size_t numsides;
			const BrushSide* sides;

		public:
			MOHPC_ASSETS_EXPORTS const Shader* GetShader() const;
			MOHPC_ASSETS_EXPORTS int32_t GetContents() const;
			MOHPC_ASSETS_EXPORTS const_vec3p_t GetMins() const;
			MOHPC_ASSETS_EXPORTS const_vec3p_t GetMaxs() const;
			MOHPC_ASSETS_EXPORTS size_t GetNumSides() const;
			MOHPC_ASSETS_EXPORTS const BrushSide* GetSide(size_t Index) const;
			MOHPC_ASSETS_EXPORTS void GetOrigin(vec3r_t out) const;
		};

		struct Node
		{
			const Plane* plane;
			int32_t children[2];
		};

		struct Leaf
		{
			int32_t cluster;
			int32_t area;
			uintptr_t firstLeafBrush;
			uintptr_t numLeafBrushes;
			uintptr_t firstLeafSurface;
			uintptr_t numLeafSurfaces;
			uintptr_t firstLeafTerrain;
			uintptr_t numLeafTerrains;
		};

		struct Area
		{
			uint32_t floodNum;
			uint32_t floodValid;
		};

		struct Model
		{
			vec3_t bounds[2];
			const Surface* surface;
			int32_t numSurfaces;
			Leaf leaf;
		};

		struct SphereLight
		{
			vec3_t origin;
			vec3_t color;
			float intensity;
			bool bNeedsTrace;
			bool bSpotLight;
			float spotRadiusByDistance;
			vec3_t spotDirection;
		};

		struct StaticModel
		{
			fs::path modelName;
			vec3_t origin;
			vec3_t angles;
			float scale;
			int32_t firstVertexData;
			int32_t numVertexData;
			int32_t visCount;
			//dtiki_t *tiki;
			//sphere_dlight_t dlights[32];
			//int numdlights;
			float radius;
		};

		union Varnode {
			float variance;
			struct {
				uint8_t flags;
				uint8_t unused[3];
			} s;
		};

		struct TerrainSurface
		{
		public:
			TerrainSurface();

		public:
			uint8_t* lmapData;
			int32_t numVerts;
			int32_t numTris;
			int32_t lmapSize;
			int32_t dlightBits[2];
			int32_t dlightmap[2];
			float lmapStep;
			float lmapX;
			float lmapY;
			terraInt vertHead;
			terraInt triHead;
			terraInt triTail;
			terraInt mergeHead;
		};

		struct TerrainPatchDrawInfo
		{
		public:
			TerrainPatchDrawInfo();

		public:
			TerrainSurface drawInfo;
			int32_t viewCount;
			int32_t visCountCheck;
			int32_t visCountDraw;
			int32_t frameCount;
			uint32_t distRecalc;
		};

		struct TerrainPatch
		{
			const Shader* shader;
			float s;
			float t;
			float texCoord[2][2][2];
			float x0;
			float y0;
			float z0;
			float zmax;
			int16_t north;
			int16_t east;
			int16_t south;
			int16_t west;
			Varnode varTree[2][63];
			uint8_t heightmap[81];
			uint8_t flags;
		};

		struct TerrainVert
		{
		public:
			TerrainVert();

		public:
			const uint8_t* pHgt;
			float fVariance;
			float fHgtAvg;
			float fHgtAdd;
			vec3_t xyz;
			vec2_t texCoords[2];
			uint32_t uiDistRecalc;
			terraInt nRef;
			terraInt iVertArray;
			terraInt iNext;
			terraInt iPrev;
		};

		struct TerrainTri
		{
		public:
			TerrainTri();

		public:
			const TerrainPatch* patch;
			const Varnode* varnode;
			TerrainPatchDrawInfo* info;
			uint32_t uiDistRecalc;
			terraInt iPt[3];
			terraInt nSplit;
			terraInt index;
			terraInt iLeft;
			terraInt iRight;
			terraInt iBase;
			terraInt iLeftChild;
			terraInt iRightChild;
			terraInt iParent;
			terraInt iPrev;
			terraInt iNext;
			uint8_t lod;
			uint8_t byConstChecks;
		};

		struct PoolInfo
		{
		public:
			PoolInfo();

		public:
			terraInt iFreeHead;
			terraInt iCur;
			size_t nFree;
		};
	}

	class BSP : public Asset
	{
		MOHPC_ASSET_OBJECT_DECLARATION(BSP);

	public:
		MOHPC_ASSETS_EXPORTS BSP(const fs::path& path);
		~BSP();

		/** Returns the number of shaders. */
		MOHPC_ASSETS_EXPORTS size_t GetNumShaders() const;

		/** Returns the BSPData::Shader at the specified number. */
		MOHPC_ASSETS_EXPORTS const BSPData::Shader *GetShader(size_t shaderNum) const;

		/** Returns the number of lightmaps. */
		MOHPC_ASSETS_EXPORTS size_t GetNumLightmaps() const;

		/** Returns the lightmap at the specified number. */
		MOHPC_ASSETS_EXPORTS const BSPData::Lightmap* GetLightmap(size_t lightmapNum) const;

		/** Returns the number of surfaces. */
		MOHPC_ASSETS_EXPORTS size_t GetNumSurfaces() const;

		/** Returns the surface at the specified number. */
		MOHPC_ASSETS_EXPORTS const BSPData::Surface *GetSurface(size_t surfaceNum);

		/** Returns the number of planes. */
		MOHPC_ASSETS_EXPORTS size_t GetNumPlanes() const;

		/** Returns the plane at the specified number. */
		MOHPC_ASSETS_EXPORTS const BSPData::Plane *GetPlane(size_t planeNum);

		/** Returns the number of side equations. */
		MOHPC_ASSETS_EXPORTS size_t GetNumSideEquations() const;

		/** Returns the side equation at the specified number. */
		MOHPC_ASSETS_EXPORTS const BSPData::SideEquation *GetSideEquation(size_t equationNum);

		/** Returns the number of brush sides. */
		MOHPC_ASSETS_EXPORTS size_t GetNumBrushSides() const;

		/** Returns the brush side at the specified number. */
		MOHPC_ASSETS_EXPORTS const BSPData::BrushSide *GetBrushSide(size_t brushSideNum);

		/** Returns the number of brushes. */
		MOHPC_ASSETS_EXPORTS size_t GetNumBrushes() const;

		/** Returns the brush at the specified number. */
		MOHPC_ASSETS_EXPORTS const BSPData::Brush* GetBrush(size_t brushNum) const;

		/** Returns the number of leafs. */
		MOHPC_ASSETS_EXPORTS size_t GetNumLeafs() const;

		/** Returns the leaf at the specified number. */
		MOHPC_ASSETS_EXPORTS const BSPData::Leaf* GetLeaf(size_t leafNum) const;

		/** Returns the number of nodes. */
		MOHPC_ASSETS_EXPORTS size_t GetNumNodes() const;

		/** Returns the node at the specified number. */
		MOHPC_ASSETS_EXPORTS const BSPData::Node* GetNode(size_t nodeNum) const;

		/** Returns the number of leaf brushes. */
		MOHPC_ASSETS_EXPORTS size_t GetNumLeafBrushes() const;

		/** Returns the leaf brush at the specified number. */
		MOHPC_ASSETS_EXPORTS uintptr_t GetLeafBrush(size_t leafBrushNum) const;

		/** Returns the number of leaf surfaces. */
		MOHPC_ASSETS_EXPORTS size_t GetNumLeafSurfaces() const;

		/** Returns the leaf surface at the specified number. */
		MOHPC_ASSETS_EXPORTS uintptr_t GetLeafSurface(size_t leafSurfNum) const;

		/** Returns the number of submodels. */
		MOHPC_ASSETS_EXPORTS size_t GetNumSubmodels() const;

		/** Returns the leaf surface at the specified number. */
		MOHPC_ASSETS_EXPORTS const BSPData::TerrainPatch* GetLeafTerrain(size_t leafTerrainNum) const;

		/** Returns the number of submodels. */
		MOHPC_ASSETS_EXPORTS size_t GetNumLeafTerrains() const;

		/** Returns the submodel at the specified number. */
		MOHPC_ASSETS_EXPORTS const BSPData::Model *GetSubmodel(size_t submodelNum) const;

		/** Returns the submodel by name (in the format '*x'). */
		MOHPC_ASSETS_EXPORTS const BSPData::Model *GetSubmodel(const str& submodelName) const;

		/** Returns the number of sphere lights. */
		MOHPC_ASSETS_EXPORTS size_t GetNumLights() const;

		/** Returns the light at the specified number. */
		MOHPC_ASSETS_EXPORTS const BSPData::SphereLight *GetLight(size_t lightNum) const;

		/** Returns the number of static models. */
		MOHPC_ASSETS_EXPORTS size_t GetNumStaticModels() const;

		/** Returns the static model at the specified number. */
		MOHPC_ASSETS_EXPORTS const BSPData::StaticModel *GetStaticModel(size_t staticModelNum) const;

		/** Returns the number of terrain patches. */
		MOHPC_ASSETS_EXPORTS size_t GetNumTerrainPatches() const;

		/** Returns the terrain patch at the specified number. */
		MOHPC_ASSETS_EXPORTS const BSPData::TerrainPatch *GetTerrainPatch(size_t terrainPatchNum) const;

		/** Returns the number of terrain surfaces. */
		MOHPC_ASSETS_EXPORTS size_t GetNumTerrainSurfaces() const;

		/** Returns the terrain surface at the specified number. */
		MOHPC_ASSETS_EXPORTS const BSPData::Surface *GetTerrainSurface(size_t terrainSurfaceNum) const;

		/** Returns the number of entities in this level (static models are not counted). */
		MOHPC_ASSETS_EXPORTS size_t GetNumEntities() const;

		/** Returns the entity at the specified number. */
		MOHPC_ASSETS_EXPORTS const class LevelEntity *GetEntity(size_t entityNum) const;

		/** Returns the first entity found with the targetname. */
		MOHPC_ASSETS_EXPORTS const class LevelEntity* GetEntity(const str& targetName) const;

		/** Returns an array of entities with the specified targetname. */
		MOHPC_ASSETS_EXPORTS const std::vector<class LevelEntity *>* GetEntities(const str& targetName) const;

		/** Return the leaf number of the point at the specified location. */
		MOHPC_ASSETS_EXPORTS uintptr_t PointLeafNum(const vec3r_t p);

		std::vector<BSPData::Shader>& getShaders();
		std::vector<BSPData::Lightmap>& getLightmaps();
		std::vector<BSPData::Surface>& getSurfaces();
		std::vector<BSPData::Plane>& getPlanes();
		std::vector<BSPData::SideEquation>& getSideEquations();
		std::vector<BSPData::BrushSide>& getBrushSides();
		std::vector<BSPData::Brush>& getBrushes();
		std::vector<BSPData::Node>& getNodes();
		std::vector<BSPData::Leaf>& getLeafs();
		std::vector<uintptr_t>& getLeafBrushes();
		std::vector<uintptr_t>& getLeafSurfaces();
		std::vector<const BSPData::TerrainPatch*>& getLeafTerrains();
		std::vector<BSPData::Area>& getAreas();
		std::vector<uintptr_t>& getAreaPortals();
		std::vector<BSPData::Model>& getBrushModels();
		std::vector<BSPData::SphereLight>& getSphereLights();
		std::vector<BSPData::StaticModel>& getStaticModels();
		std::vector<BSPData::TerrainPatch>& getTerrainPatches();
		std::vector<BSPData::Surface>& getTerrainSurfaces();
		std::vector<class LevelEntity*>& getEntities();
		std::unordered_map<str, std::vector<class LevelEntity*>>& getTargetList();
		std::vector<uint8_t>& getVisibility();

		void setNumClusters(uint32_t numClustersValue);
		void setNumAreas(uint32_t numAreasValue);

	private:
		uintptr_t PointLeafNum_r(const vec3r_t p, intptr_t num);

	private:
		std::vector<BSPData::Shader> shaders;
		std::vector<BSPData::Lightmap> lightmaps;
		std::vector<BSPData::Surface> surfaces;
		std::vector<BSPData::Plane> planes;
		std::vector<BSPData::SideEquation> sideEquations;
		std::vector<BSPData::BrushSide> brushSides;
		std::vector<BSPData::Brush> brushes;
		std::vector<BSPData::Node> nodes;
		std::vector<BSPData::Leaf> leafs;
		std::vector<uintptr_t> leafBrushes;
		std::vector<uintptr_t> leafSurfaces;
		std::vector<const BSPData::TerrainPatch*> leafTerrains;
		std::vector<BSPData::Area> areas;
		std::vector<uintptr_t> areaPortals;
		std::vector<BSPData::Model> brushModels;
		std::vector<BSPData::SphereLight> lights;
		std::vector<BSPData::StaticModel> staticModels;
		std::vector<BSPData::TerrainPatch> terrainPatches;
		std::vector<BSPData::Surface> terrainSurfaces;
		std::vector<class LevelEntity*> entities;
		std::unordered_map<str, std::vector<class LevelEntity*>> targetList;
		std::vector<uint8_t> visibility;
		uint32_t numClusters;
		uint32_t numAreas;
		uint32_t clusterBytes;
	};
	using BSPPtr = SharedPtr<BSP>;

	struct patchWork_t
	{
		uint32_t numPlanes;
		uint32_t numFacets;
		BSPData::PatchPlane planes[4096];
		BSPData::Facet facets[1024];
	};

	class BSPReader : public AssetReader
	{
		MOHPC_ASSET_OBJECT_DECLARATION(BSPReader);

	public:
		using AssetType = BSP;

	public:
		MOHPC_ASSETS_EXPORTS BSPReader();
		MOHPC_ASSETS_EXPORTS ~BSPReader();

		MOHPC_ASSETS_EXPORTS AssetPtr read(const IFilePtr& file) override;

	private:

	private:
		//void PreAllocateLevelData(const File_Header *Header);
		void LoadShaders(const BSPFile::GameLump* GameLump, std::vector<BSPData::Shader>& shaders);
		void LoadLightmaps(const BSPFile::GameLump* GameLump, std::vector<BSPData::Lightmap>& lightmaps);

		BSPData::PatchCollide* GeneratePatchCollide(int32_t width, int32_t height, const BSPData::Vertice* points, float subdivisions);
		void CreateSurfaceGridMesh(int32_t width, int32_t height, BSPData::Vertice* ctrl, int32_t numIndexes, int32_t* indexes, BSPData::Surface* grid);
		void SubdividePatchToGrid(int32_t Width, int32_t Height, const BSPData::Vertice* Points, BSPData::Surface* Out);
		void ParseMesh(const BSPFile::fsurface_t* InSurface, const BSPFile::fvertice_t* InVertices, const std::vector<BSPData::Shader>& shaders, BSPData::Surface* Out);
		void ParseFace(const BSPFile::fsurface_t* InSurface, const BSPFile::fvertice_t* InVertices, const int32_t* InIndices, const std::vector<BSPData::Shader>& shaders, BSPData::Surface* Out);
		void ParseTriSurf(const BSPFile::fsurface_t* InSurface, const BSPFile::fvertice_t* InVertices, const int32_t* InIndices, const std::vector<BSPData::Shader>& shaders, BSPData::Surface* Out);
		void LoadSurfaces(
			const BSPFile::GameLump* SurfacesLump,
			const BSPFile::GameLump* Vertices,
			BSPFile::GameLump* Indices,
			const std::vector<BSPData::Shader>& shaders,
			std::vector<BSPData::Surface>& surfaces
		);

		void LoadPlanes(const BSPFile::GameLump* GameLump, std::vector<BSPData::Plane>& planes);
		void LoadSideEquations(const BSPFile::GameLump* GameLump, std::vector<BSPData::SideEquation>& sideEquations);
		void LoadBrushSides(
			const BSPFile::GameLump* GameLump,
			const std::vector<BSPData::Shader>& shaders,
			const std::vector<BSPData::Plane>& planes,
			const std::vector<BSPData::SideEquation>& sideEquations,
			std::vector<BSPData::BrushSide>& brushSides
		);
		void LoadBrushes(
			const BSPFile::GameLump* GameLump,
			const std::vector<BSPData::Shader>& shaders,
			const std::vector<BSPData::BrushSide>& brushSides,
			std::vector<BSPData::Brush>& brushes
		);
		void LoadLeafs(
			const BSPFile::GameLump* GameLump,
			std::vector<BSPData::Leaf>& leafs,
			std::vector<BSPData::Area>& areas,
			std::vector<uintptr_t>& areaPortals,
			uint32_t& numClusters,
			uint32_t& numAreas
		);

		void LoadLeafsOld(
			const BSPFile::GameLump* GameLump,
			std::vector<BSPData::Leaf>& leafs,
			std::vector<BSPData::Area>& areas,
			std::vector<uintptr_t>& areaPortals,
			uint32_t& numClusters,
			uint32_t& numAreas
		);
		void LoadLeafsBrushes(const BSPFile::GameLump* GameLump, std::vector<uintptr_t>& leafBrushes);
		void LoadLeafSurfaces(const BSPFile::GameLump* GameLump, std::vector<uintptr_t>& leafSurfaces);
		void LoadNodes(const BSPFile::GameLump* GameLump, const std::vector<BSPData::Plane>& planes, std::vector<BSPData::Node>& nodes);
		void LoadVisibility(const BSPFile::GameLump* GameLump, std::vector<uint8_t>& visibility, uint32_t& numClusters, uint32_t& clusterBytes);
		void LoadSubmodels(
			const BSPFile::GameLump* GameLump,
			const std::vector<BSPData::Surface>& surfaces,
			std::vector<uintptr_t>& leafBrushes,
			std::vector<uintptr_t>& leafSurfaces,
			std::vector<BSPData::Model>& brushModels
		);

		void LoadEntityString(const BSPFile::GameLump* GameLump, char*& entityString, size_t& entityStringLength);
		void LoadSphereLights(const BSPFile::GameLump* GameLump, std::vector<BSPData::SphereLight>& lights);
		void LoadStaticModelDefs(const BSPFile::GameLump* GameLump, std::vector<BSPData::StaticModel>& staticModels);
		void LoadTerrain(const BSPFile::GameLump* GameLump, const std::vector<BSPData::Shader>& shaders, std::vector<BSPData::TerrainPatch>& terrainPatches, std::vector<BSPData::TerrainPatchDrawInfo>& infoList);
		void LoadTerrainIndexes(
			const BSPFile::GameLump* GameLump,
			const std::vector<BSPData::TerrainPatch>& terrainPatches,
			std::vector<const BSPData::TerrainPatch*>& leafTerrains
		);
		void FloodArea(std::vector<BSPData::Area>& areas, const std::vector<uintptr_t>& areaPortals, uint32_t numAreas, uint32_t areaNum, uint32_t floodNum, uint32_t& floodValid);
		void FloodAreaConnections(std::vector<BSPData::Area>& areas, const std::vector<uintptr_t>& areaPortals, uint32_t numAreas);

		// terrain
		terraInt TR_AllocateVert(const BSPData::TerrainPatch* patch, BSPData::TerrainPatchDrawInfo& info);
		void TR_InterpolateVert(BSPData::TerrainTri* pTri, BSPData::TerrainVert* pVert);
		void TR_ReleaseVert(const BSPData::TerrainPatch* patch, BSPData::TerrainPatchDrawInfo& info, terraInt iVert);
		terraInt TR_AllocateTri(const BSPData::TerrainPatch* patch, BSPData::TerrainPatchDrawInfo& info, uint8_t byConstChecks = 4);
		void TR_FixTriHeight(BSPData::TerrainTri* pTri);
		void TR_SetTriConstChecks(BSPData::TerrainTri* pTri);
		void TR_ReleaseTri(const BSPData::TerrainPatch* patch, BSPData::TerrainPatchDrawInfo& info, terraInt iTri);
		void TR_DemoteInAncestry(const BSPData::TerrainPatch* patch, BSPData::TerrainPatchDrawInfo& info, terraInt iTri);
		void TR_TerrainHeapInit();
		void TR_SplitTri(terraInt iSplit, terraInt iNewPt, terraInt iLeft, terraInt iRight, terraInt iRightOfLeft, terraInt iLeftOfRight);
		void TR_ForceSplit(terraInt iTri);
		void TR_ForceMerge(terraInt iTri);
		int TR_TerraTriNeighbor(const BSPData::TerrainPatch* terraPatches, int iPatch, int dir);
		void TR_PreTessellateTerrain(const std::vector<BSPData::TerrainPatch>& terrainPatches, std::vector<BSPData::TerrainPatchDrawInfo>& infoList);
		bool TR_NeedSplitTri(BSPData::TerrainTri* pTri);
		void TR_DoTriSplitting(const std::vector<BSPData::TerrainPatch>& terrainPatches, std::vector<BSPData::TerrainPatchDrawInfo>& infoList);
		void TR_DoGeomorphs();
		bool TR_MergeInternalAggressive();
		bool TR_MergeInternalCautious();
		void TR_DoTriMerging(const std::vector<BSPData::TerrainPatch>& terrainPatches, std::vector<BSPData::TerrainPatchDrawInfo>& infoList);
		void TR_ShrinkData();

		void GenerateTerrainPatch(const BSPData::TerrainPatch* Patch, BSPData::Surface* Out);
		void GenerateTerrainPatch2(const BSPData::TerrainPatch* Patch, BSPData::TerrainPatchDrawInfo& info, BSPData::Surface* Out);
		void UnpackTerraPatch(const BSPFile::fterrainPatch_t* Packed, BSPData::TerrainPatch* Unpacked, BSPData::TerrainPatchDrawInfo& info, const std::vector<BSPData::Shader>& shaders) const;

		void BoundBrush(BSPData::Brush* Brush);
		void ColorShiftLightingFloats(float in[4], float out[4], float scale);
		void ColorShiftLightingFloats3(vec3_t in, vec3_t out, float scale);

		void CreateTerrainSurfaces(const std::vector<BSPData::TerrainPatch>& terrainPatches, std::vector<BSPData::TerrainPatchDrawInfo>& info, std::vector<BSPData::Surface>& terrainSurfaces);
		void CreateEntities(
			const char* entityString,
			size_t length,
			std::vector<class LevelEntity*>& entities,
			std::unordered_map<str, std::vector<class LevelEntity*>>& targetList
		);

		uint32_t LoadLump(const IFilePtr& file, BSPFile::flump_t* lump, BSPFile::GameLump* gameLump, size_t size = 0);

	private:
		std::vector<BSPData::TerrainVert> trVerts;
		std::vector<BSPData::TerrainTri> trTris;
		BSPData::PoolInfo trpiTri;
		BSPData::PoolInfo trpiVert;
	};

	BSPData::Plane::PlaneType PlaneTypeForNormal(const_vec3r_t Normal);
	bool PlaneFromPoints(vec4_t plane, vec3_t a, vec3_t b, vec3_t c);

	namespace BSPError
	{
		class Base : public std::exception {};

		/**
		 * BSP has wrong header.
		 */
		class BadHeader : public Base
		{
		public:
			BadHeader(const uint8_t foundHeader[4]);

			MOHPC_ASSETS_EXPORTS const uint8_t* getHeader() const;

		public:
			const char* what() const noexcept override;

		private:
			uint8_t foundHeader[4];
		};

		/**
		 * BSP has wrong or unsupported version.
		 */
		class WrongVersion : public Base
		{
		public:
			WrongVersion(uint32_t inVersion);

			MOHPC_ASSETS_EXPORTS uint32_t getVersion() const;

		public:
			const char* what() const noexcept override;

		private:
			uint32_t version;
		};

		/**
		 * Lump size doesn't match with structure size.
		 */
		class FunnyLumpSize : public Base
		{
		public:
			FunnyLumpSize(const char* inLumpName);

			MOHPC_ASSETS_EXPORTS const char* getLumpName() const;

		public:
			const char* what() const noexcept override;

		private:
			const char* lumpName;
		};

		/**
		 * Bad mesh size for patch.
		 */
		class BadMeshSize : public Base
		{
		public:
			BadMeshSize(int32_t inWidth, int32_t inHeight);

			MOHPC_ASSETS_EXPORTS int32_t getWidth() const;
			MOHPC_ASSETS_EXPORTS int32_t getHeight() const;

		public:
			const char* what() const noexcept override;

		private:
			int32_t width;
			int32_t height;
		};

		/**
		 * Invalid index on triangle.
		 */
		class BadFaceSurfaceIndex : public Base
		{
		public:
			BadFaceSurfaceIndex(uint32_t inIndex);

			MOHPC_ASSETS_EXPORTS uint32_t getIndex() const;

		public:
			const char* what() const noexcept override;

		private:
			uint32_t index;
		};

		/**
		 * Invalid lightmap scale on terrain.
		 */
		class BadTerrainLightmapScale : public Base
		{
		public:
			BadTerrainLightmapScale(uint8_t scale);

			MOHPC_ASSETS_EXPORTS uint8_t getScale() const;

		public:
			const char* what() const noexcept override;

		private:
			uint8_t scale;
		};

		/**
		 * Trying to flood an area that was already flooded.
		 */
		class RefloodedArea : public Base
		{
		public:
			RefloodedArea(uint32_t inAreaNum, uint32_t inFloodNum);

			MOHPC_ASSETS_EXPORTS uint32_t getAreaNum() const;
			MOHPC_ASSETS_EXPORTS uint32_t getFloodNum() const;

		public:
			const char* what() const noexcept override;

		private:
			uint32_t areaNum;
			uint32_t floodNum;
		};

		/**
		 * BSP has bad entity string.
		 */
		class ExpectedInitBrace : public Base
		{
		public:
			ExpectedInitBrace(char inChar);

			MOHPC_ASSETS_EXPORTS char getCharacter() const;

		public:
			const char* what() const noexcept override;

		private:
			char c ;
		};

		/** Unexpected end of file met while parsing entities. */
		class UnexpectedEntityEOF : public Base { using Base::Base; };
	}
}
