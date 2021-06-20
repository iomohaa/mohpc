#pragma once

#include "../Asset.h"
#include "../Managers/ShaderManager.h"
#include "../../Files/Managers/FileManager.h"
#include "../../Common/str.h"
#include "../../Common/Vector.h"
#include "../../Utility/SharedPtr.h"

#include <morfuse/Container/Container.h>
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

	using terraInt = int32_t;

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

			Vector normal;
			float distance;
			PlaneType type;
			uint8_t signBits;
		};

		struct PatchPlane
		{
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

		struct MOHPC_ASSETS_EXPORTS PatchCollide
		{
			Vector bounds[2];
			int32_t numPlanes;
			PatchPlane* planes;
			int32_t numFacets;
			Facet* facets;

		public:
			~PatchCollide();
		};

		struct Vertice
		{
			Vector xyz;
			float st[2];
			float lightmap[2];
			Vector normal;
			uint8_t color[4];
		};

		class Lightmap
		{
			friend BSP;

		private:
			uint8_t color[lightmapSize * lightmapSize][3];

		public:
			/** Returns the number of pixels in the lightmap. */
			MOHPC_ASSETS_EXPORTS size_t GetNumPixels() const;

			/** Returns the width of the lightmap. */
			MOHPC_ASSETS_EXPORTS size_t GetWidth() const;

			/** Returns the height of the lightmap. */
			MOHPC_ASSETS_EXPORTS size_t GetHeight() const;

			/** Returns the RGB color of the specified pixel. */
			MOHPC_ASSETS_EXPORTS void GetColor(size_t pixelNum, uint8_t(&out)[3]) const;
		};

		class Surface
		{
			friend BSP;

		private:
			const Shader* shader;
			mfuse::con::Container<Vertice> vertices;
			mfuse::con::Container<uint32_t> indexes;
			Vector centroid;
			bool bIsPatch;
			int32_t lightmapNum;
			int32_t lightmapX;
			int32_t lightmapY;
			int32_t lightmapWidth;
			int32_t lightmapHeight;
			Vector lightmapOrigin;
			Vector lightmapVecs[3];
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
				Vector bounds[2];
				Vector localOrigin;
				float radius;
				Plane plane;
			} cullInfo;

		public:
			Surface();
			~Surface();

			MOHPC_ASSETS_EXPORTS const Shader* GetShader() const;

			MOHPC_ASSETS_EXPORTS size_t GetNumVertices() const;
			MOHPC_ASSETS_EXPORTS const Vertice* GetVertice(size_t index) const;

			MOHPC_ASSETS_EXPORTS size_t GetNumIndexes() const;
			MOHPC_ASSETS_EXPORTS uint32_t GetIndice(size_t index) const;

			MOHPC_ASSETS_EXPORTS int32_t GetLightmapNum() const;
			MOHPC_ASSETS_EXPORTS int32_t GetLightmapX() const;
			MOHPC_ASSETS_EXPORTS int32_t GetLightmapY() const;
			MOHPC_ASSETS_EXPORTS int32_t GetLightmapWidth() const;
			MOHPC_ASSETS_EXPORTS int32_t GetLightmapHeight() const;
			MOHPC_ASSETS_EXPORTS const Vector& GetLightmapOrigin() const;
			MOHPC_ASSETS_EXPORTS const Vector& GetLightmapVec(int32_t num) const;
			MOHPC_ASSETS_EXPORTS const PatchCollide* GetPatchCollide() const;

			MOHPC_ASSETS_EXPORTS bool IsPatch() const;

		private:
			void CalculateCentroid();
		};

		struct SideEquation
		{
			float sEq[4];
			float tEq[4];
		};

		struct BrushSide
		{
			Plane* plane;
			int32_t surfaceFlags;
			const Shader* shader;
			SideEquation* Eq;
		};

		struct Brush
		{
			str name;
			const Shader* shader;
			int32_t contents;
			Vector bounds[2];
			size_t numsides;
			BrushSide* sides;
			Brush* parent;
			mfuse::con::Container<const Surface*> surfaces;

		public:
			MOHPC_ASSETS_EXPORTS const char* GetName() const;
			MOHPC_ASSETS_EXPORTS const Shader* GetShader() const;
			MOHPC_ASSETS_EXPORTS int32_t GetContents() const;
			MOHPC_ASSETS_EXPORTS const Vector& GetMins() const;
			MOHPC_ASSETS_EXPORTS const Vector& GetMaxs() const;
			MOHPC_ASSETS_EXPORTS size_t GetNumSides() const;
			MOHPC_ASSETS_EXPORTS BrushSide* GetSide(size_t Index) const;
			MOHPC_ASSETS_EXPORTS Brush* GetParent() const;
			MOHPC_ASSETS_EXPORTS Vector GetOrigin() const;
		};

		struct Node
		{
			Plane* plane;
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

		class SurfacesGroup
		{
			friend BSP;

		private:
			str name;
			mfuse::con::Container<const Surface*> surfaces;
			mfuse::con::Container<const Brush*> brushes;
			Vector bounds[2];
			Vector origin;

		public:
			MOHPC_ASSETS_EXPORTS const str& GetGroupName() const;
			MOHPC_ASSETS_EXPORTS size_t GetNumSurfaces() const;
			MOHPC_ASSETS_EXPORTS const Surface* GetSurface(size_t index) const;
			MOHPC_ASSETS_EXPORTS size_t GetNumBrushes() const;
			MOHPC_ASSETS_EXPORTS const Brush* GetBrush(size_t index) const;
			MOHPC_ASSETS_EXPORTS const Surface* const* GetSurfaces() const;
			MOHPC_ASSETS_EXPORTS const Brush* const* GetBrushes() const;
			MOHPC_ASSETS_EXPORTS const Vector& GetMinBound() const;
			MOHPC_ASSETS_EXPORTS const Vector& GetMaxBound() const;
			MOHPC_ASSETS_EXPORTS const Vector& GetOrigin() const;
		};

		struct Model
		{
			Vector bounds[2];
			Surface* surface;
			int32_t numSurfaces;
			Leaf leaf;
		};

		struct SphereLight
		{
			Vector origin;
			Vector color;
			float intensity;
			bool bNeedsTrace;
			bool bSpotLight;
			float spotRadiusByDistance;
			Vector spotDirection;
		};

		struct StaticModel
		{
			str modelName;
			Vector origin;
			Vector angles;
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
			terraInt vertHead;
			terraInt triHead;
			terraInt triTail;
			terraInt mergeHead;
			int32_t numVerts;
			int32_t numTris;
			int32_t lmapSize;
			int32_t dlightBits[2];
			float lmapStep;
			int32_t dlightmap[2];
			uint8_t* lmapData;
			float lmapX;
			float lmapY;
		};

		struct TerrainPatch
		{
			TerrainSurface drawInfo;
			int32_t viewCount;
			int32_t visCountCheck;
			int32_t visCountDraw;
			int32_t frameCount;
			uint32_t distRecalc;
			float s;
			float t;
			float texCoord[2][2][2];
			float x0;
			float y0;
			float z0;
			float zmax;
			const Shader* shader;
			int16_t north;
			int16_t east;
			int16_t south;
			int16_t west;
			TerrainPatch* nextActive;
			Varnode varTree[2][63];
			uint8_t heightmap[81];
			uint8_t flags;
			bool bByDirty;
		};

		struct TerrainVert
		{
			vec3_t xyz;
			vec2_t texCoords[2];
			float fVariance;
			float fHgtAvg;
			float fHgtAdd;
			unsigned int uiDistRecalc;
			terraInt nRef;
			terraInt iVertArray;
			uint8_t* pHgt;
			terraInt iNext;
			terraInt iPrev;

			TerrainVert();
		};

		struct TerrainTri
		{
			terraInt iPt[3];
			terraInt nSplit;
			unsigned int uiDistRecalc;
			TerrainPatch* patch;
			Varnode* varnode;
			terraInt index;
			uint8_t lod;
			uint8_t byConstChecks;
			terraInt iLeft;
			terraInt iRight;
			terraInt iBase;
			terraInt iLeftChild;
			terraInt iRightChild;
			terraInt iParent;
			terraInt iPrev;
			terraInt iNext;

			TerrainTri();
		};

		struct TerrainCollideSquare {
			vec4_t plane[2];
			int32_t eMode;
		};

		struct TerrainCollide {
			Vector vBounds[2];
			TerrainCollideSquare squares[8][8];
		};

		struct PoolInfo
		{
			terraInt iFreeHead;
			terraInt iCur;
			size_t nFree;

			PoolInfo()
				: iFreeHead(0)
				, iCur(0)
				, nFree(0)
			{
			}
		};

		struct varnodeIndex {
			short unsigned int iTreeAndMask;
			short unsigned int iNode;
		};
	}

	class BSP : public Asset
	{
		MOHPC_ASSET_OBJECT_DECLARATION(BSP);

	public:
		MOHPC_ASSETS_EXPORTS BSP();
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
		MOHPC_ASSETS_EXPORTS const mfuse::con::Container<class LevelEntity *>* GetEntities(const str& targetName) const;

		/** Returns the number of grouped surfaces. */
		MOHPC_ASSETS_EXPORTS size_t GetNumSurfacesGroup() const;

		/** Returns the grouped surface at the specified number. */
		MOHPC_ASSETS_EXPORTS const BSPData::SurfacesGroup *GetSurfacesGroup(size_t surfsGroupNum) const;

		/** Generate planes of collision from a terrain patch. */
		MOHPC_ASSETS_EXPORTS void GenerateTerrainCollide(const BSPData::TerrainPatch* patch, BSPData::TerrainCollide& collision);

		/** Return the leaf number of the point at the specified location. */
		MOHPC_ASSETS_EXPORTS uintptr_t PointLeafNum(const MOHPC::Vector p);

		/** Fill the specified collision world for tracing, etc... */
		MOHPC_ASSETS_EXPORTS void FillCollisionWorld(CollisionWorld& cm);

		static bool PlaneFromPoints(vec4_t plane, vec3_t a, vec3_t b, vec3_t c);

	protected:
		void Load() override;

	private:
		BSPData::Plane::PlaneType PlaneTypeForNormal(const Vector& Normal);
		uintptr_t PointLeafNum_r(const MOHPC::Vector p, intptr_t num);

		//void PreAllocateLevelData(const File_Header *Header);
		void LoadShaders(const BSPFile::GameLump* GameLump);
		void LoadLightmaps(const BSPFile::GameLump* GameLump);

		void CreateSurfaceGridMesh(int32_t width, int32_t height, BSPData::Vertice *ctrl, int32_t numIndexes, int32_t *indexes, BSPData::Surface* grid);
		void SubdividePatchToGrid(int32_t Width, int32_t Height, const BSPData::Vertice* Points, BSPData::Surface* Out);
		BSPData::PatchCollide* GeneratePatchCollide(int32_t width, int32_t height, const BSPData::Vertice *points, float subdivisions);
		void ParseMesh(const BSPFile::fsurface_t* InSurface, const BSPFile::fvertice_t* InVertices, BSPData::Surface* Out);
		void ParseFace(const BSPFile::fsurface_t* InSurface, const BSPFile::fvertice_t* InVertices, const int32_t* InIndices, BSPData::Surface* Out);
		void ParseTriSurf(const BSPFile::fsurface_t* InSurface, const BSPFile::fvertice_t* InVertices, const int32_t* InIndices, BSPData::Surface* Out);
		void LoadSurfaces(const BSPFile::GameLump* Surfaces, const BSPFile::GameLump* Vertices, const BSPFile::GameLump* Indices);

		void LoadPlanes(const BSPFile::GameLump* GameLump);
		void LoadSideEquations(const BSPFile::GameLump* GameLump);
		void LoadBrushSides(const BSPFile::GameLump* GameLump);
		void LoadBrushes(const BSPFile::GameLump* GameLump);
		void LoadLeafs(const BSPFile::GameLump* GameLump);
		void LoadLeafsOld(const BSPFile::GameLump* GameLump);
		void LoadLeafsBrushes(const BSPFile::GameLump* GameLump);
		void LoadLeafSurfaces(const BSPFile::GameLump* GameLump);
		void LoadNodes(const BSPFile::GameLump* GameLump);
		void LoadVisibility(const BSPFile::GameLump* GameLump);
		void LoadSubmodels(const BSPFile::GameLump* GameLump);
		void LoadEntityString(const BSPFile::GameLump* GameLump);
		void LoadSphereLights(const BSPFile::GameLump* GameLump);
		void LoadStaticModelDefs(const BSPFile::GameLump* GameLump);
		void LoadTerrain(const BSPFile::GameLump* GameLump);
		void LoadTerrainIndexes(const BSPFile::GameLump* GameLump);
		void FloodArea(uint32_t areaNum, uint32_t floodNum, uint32_t& floodValid);
		void FloodAreaConnections();

		// terrain
		terraInt TR_AllocateVert(BSPData::TerrainPatch *patch);
		void TR_InterpolateVert(BSPData::TerrainTri *pTri, BSPData::TerrainVert *pVert);
		void TR_ReleaseVert(BSPData::TerrainPatch *patch, terraInt iVert);
		terraInt TR_AllocateTri(BSPData::TerrainPatch *patch, uint8_t byConstChecks = 4);
		void TR_FixTriHeight(BSPData::TerrainTri* pTri);
		void TR_SetTriConstChecks(BSPData::TerrainTri* pTri);
		void TR_ReleaseTri(BSPData::TerrainPatch *patch, terraInt iTri);
		void TR_DemoteInAncestry(BSPData::TerrainPatch *patch, terraInt iTri);
		void TR_TerrainHeapInit();
		void TR_SplitTri(terraInt iSplit, terraInt iNewPt, terraInt iLeft, terraInt iRight, terraInt iRightOfLeft, terraInt iLeftOfRight);
		void TR_ForceSplit(terraInt iTri);
		void TR_ForceMerge(terraInt iTri);
		int TR_TerraTriNeighbor(BSPData::TerrainPatch *terraPatches, int iPatch, int dir);
		void TR_PreTessellateTerrain();
		bool TR_NeedSplitTri(BSPData::TerrainTri *pTri);
		void TR_DoTriSplitting();
		void TR_DoGeomorphs();
		bool TR_MergeInternalAggressive();
		bool TR_MergeInternalCautious();
		void TR_DoTriMerging();
		void TR_ShrinkData();

		// terrain collision
		void TR_CalculateTerrainIndices(worknode_t* worknode, int iDiagonal, int iTree);
		void TR_PrepareGerrainCollide();
		void TR_PickTerrainSquareMode(BSPData::TerrainCollideSquare* square, const MOHPC::Vector& vTest, terraInt i, terraInt j, const BSPData::TerrainPatch* patch);

		void GenerateTerrainPatch(const BSPData::TerrainPatch* Patch, BSPData::Surface* Out);
		void GenerateTerrainPatch2(const BSPData::TerrainPatch* Patch, BSPData::Surface* Out);
		void UnpackTerraPatch(const BSPFile::fterrainPatch_t* Packed, BSPData::TerrainPatch* Unpacked) const;

		void BoundBrush(BSPData::Brush* Brush);
		void ColorShiftLightingFloats(float in[4], float out[4], float scale);
		void ColorShiftLightingFloats3(vec3_t in, vec3_t out, float scale);

		void CreateTerrainSurfaces();
		void CreateEntities();
		void MapBrushes();

		uint32_t LoadLump(const FilePtr& file, BSPFile::flump_t* lump, BSPFile::GameLump* gameLump, size_t size = 0);

	private:
		mfuse::con::Container<BSPData::Shader> shaders;
		mfuse::con::Container<BSPData::Lightmap> lightmaps;
		mfuse::con::Container<BSPData::Surface> surfaces;
		mfuse::con::Container<BSPData::Plane> planes;
		mfuse::con::Container<BSPData::SideEquation> sideEquations;
		mfuse::con::Container<BSPData::BrushSide> brushSides;
		mfuse::con::Container<BSPData::Brush> brushes;
		mfuse::con::Container<BSPData::Node> nodes;
		mfuse::con::Container<BSPData::Leaf> leafs;
		mfuse::con::Container<uintptr_t> leafBrushes;
		mfuse::con::Container<uintptr_t> leafSurfaces;
		mfuse::con::Container<BSPData::TerrainPatch*> leafTerrains;
		mfuse::con::Container<BSPData::Area> areas;
		mfuse::con::Container<uintptr_t> areaPortals;
		mfuse::con::Container<BSPData::Model> brushModels;
		mfuse::con::Container<BSPData::SphereLight> lights;
		mfuse::con::Container<BSPData::StaticModel> staticModels;
		mfuse::con::Container<BSPData::TerrainPatch> terrainPatches;
		mfuse::con::Container<BSPData::Surface> terrainSurfaces;
		mfuse::con::Container<class LevelEntity*> entities;
		mfuse::con::set<str, mfuse::con::Container<class LevelEntity*>> targetList;
		mfuse::con::Container<BSPData::SurfacesGroup*> surfacesGroups;
		uint32_t numClusters;
		uint32_t numAreas;
		uint32_t clusterBytes;
		mfuse::con::Container<uint8_t> visibility;
		char* entityString;
		size_t entityStringLength;

		mfuse::con::Container<BSPData::TerrainVert> trVerts;
		mfuse::con::Container<BSPData::TerrainTri> trTris;
		BSPData::PoolInfo trpiTri;
		BSPData::PoolInfo trpiVert;
		BSPData::varnodeIndex varnodeIndexes[2][8][8][2];
	};
	using BSPPtr = SharedPtr<BSP>;

	struct patchWork_t
	{
		uint32_t numPlanes;
		uint32_t numFacets;
		BSPData::PatchPlane planes[4096];
		BSPData::Facet facets[1024];
	};

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
