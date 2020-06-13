#pragma once

#include "../Global.h"
#include "../Asset.h"
#include "../Managers/FileManager.h"
#include "../Managers/ShaderManager.h"
#include <stdint.h>
#include "../Script/Container.h"
#include "../Script/str.h"
#include "../Vector.h"
#include "../Utilities/SharedPtr.h"

namespace MOHPC
{
	struct worknode_t;

	class File;
	class Shader;
	class CollisionWorld;

	class BSP : public Asset
	{
		CLASS_BODY(BSP);

	private:
		struct File_Lump;
		struct GameLump;
		struct File_Header;
		struct File_Shader;
		struct File_Surface;
		struct File_Vertice;
		struct File_Plane;
		struct File_SideEquation;
		struct File_BrushSide;
		struct File_Brush;
		struct File_BrushModel;
		struct File_SphereLight;
		struct File_StaticModel;
		struct File_TerrainPatch;
		struct File_Node;
		struct File_Leaf;
		struct File_Leaf_Ver17;

	public:
		static const int32_t lightmapSize = 128;
		static const int32_t lightmapMemSize = lightmapSize * lightmapSize * sizeof(uint8_t) * 3;
		static const float lightmapTerrainLength;

		struct Shader
		{
			str shaderName;
			int32_t surfaceFlags;
			int32_t contentFlags;
			int32_t subdivisions;
			str fenceMaskImage;

			ShaderRef shader;
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
			int32_t signbits;
		} ;

		struct Facet
		{
			int32_t surfacePlane;
			int32_t numBorders;
			int32_t borderPlanes[4 + 6 + 16];
			int32_t borderInward[4 + 6 + 16];
			bool borderNoAdjust[4 + 6 + 16];
		};

		struct MOHPC_EXPORTS PatchCollide
		{
			Vector bounds[2];
			int32_t numPlanes;
			PatchPlane *planes;
			int32_t numFacets;
			Facet *facets;

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
			MOHPC_EXPORTS size_t GetNumPixels() const;

			/** Returns the width of the lightmap. */
			MOHPC_EXPORTS size_t GetWidth() const;

			/** Returns the height of the lightmap. */
			MOHPC_EXPORTS size_t GetHeight() const;

			/** Returns the RGB color of the specified pixel. */
			MOHPC_EXPORTS void GetColor(size_t pixelNum, uint8_t (&out)[3]) const;
		};

		class Surface
		{
			friend BSP;

		private:
			const Shader* shader;
			Container<Vertice> vertices;
			Container<size_t> indexes;
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

			MOHPC_EXPORTS const Shader* GetShader() const;

			MOHPC_EXPORTS size_t GetNumVertices() const;
			MOHPC_EXPORTS const Vertice *GetVertice(size_t index) const;

			MOHPC_EXPORTS size_t GetNumIndexes() const;
			MOHPC_EXPORTS size_t GetIndice(size_t index) const;

			MOHPC_EXPORTS int32_t GetLightmapNum() const;
			MOHPC_EXPORTS int32_t GetLightmapX() const;
			MOHPC_EXPORTS int32_t GetLightmapY() const;
			MOHPC_EXPORTS int32_t GetLightmapWidth() const;
			MOHPC_EXPORTS int32_t GetLightmapHeight() const;
			MOHPC_EXPORTS const Vector& GetLightmapOrigin() const;
			MOHPC_EXPORTS const Vector& GetLightmapVec(int32_t num) const;
			MOHPC_EXPORTS const PatchCollide* GetPatchCollide() const;

			MOHPC_EXPORTS bool IsPatch() const;

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
			Container<const Surface *> surfaces;

		public:
			MOHPC_EXPORTS const char* GetName() const;
			MOHPC_EXPORTS const Shader* GetShader() const;
			MOHPC_EXPORTS int32_t GetContents() const;
			MOHPC_EXPORTS const Vector& GetMins() const;
			MOHPC_EXPORTS const Vector& GetMaxs() const;
			MOHPC_EXPORTS size_t GetNumSides() const;
			MOHPC_EXPORTS BrushSide* GetSide(size_t Index) const;
			MOHPC_EXPORTS Brush* GetParent() const;
			MOHPC_EXPORTS Vector GetOrigin() const;
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
			Container<const Surface *> surfaces;
			Container<const Brush *> brushes;
			Vector bounds[2];
			Vector origin;

		public:
			MOHPC_EXPORTS const str& GetGroupName() const;
			MOHPC_EXPORTS size_t GetNumSurfaces() const;
			MOHPC_EXPORTS const Surface *GetSurface(size_t index) const;
			MOHPC_EXPORTS size_t GetNumBrushes() const;
			MOHPC_EXPORTS const Brush *GetBrush(size_t index) const;
			MOHPC_EXPORTS const Surface* const* GetSurfaces() const;
			MOHPC_EXPORTS const Brush* const* GetBrushes() const;
			MOHPC_EXPORTS const Vector& GetMinBound() const;
			MOHPC_EXPORTS const Vector& GetMaxBound() const;
			MOHPC_EXPORTS const Vector& GetOrigin() const;
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

		typedef int32_t terraInt;

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
			uint8_t *pHgt;
			terraInt iNext;
			terraInt iPrev;

			TerrainVert();
		};

		struct TerrainTri
		{
			terraInt iPt[3];
			terraInt nSplit;
			unsigned int uiDistRecalc;
			TerrainPatch *patch;
			Varnode *varnode;
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

	private:
		struct varnodeIndex {
			short unsigned int iTreeAndMask;
			short unsigned int iNode;
		};

	public:
		MOHPC_EXPORTS BSP();
		~BSP();

		/** Returns the number of shaders. */
		MOHPC_EXPORTS size_t GetNumShaders() const;

		/** Returns the shader at the specified number. */
		MOHPC_EXPORTS const Shader *GetShader(size_t shaderNum) const;

		/** Returns the number of lightmaps. */
		MOHPC_EXPORTS size_t GetNumLightmaps() const;

		/** Returns the lightmap at the specified number. */
		MOHPC_EXPORTS const Lightmap* GetLightmap(size_t lightmapNum) const;

		/** Returns the number of surfaces. */
		MOHPC_EXPORTS size_t GetNumSurfaces() const;

		/** Returns the surface at the specified number. */
		MOHPC_EXPORTS const Surface *GetSurface(size_t surfaceNum);

		/** Returns the number of planes. */
		MOHPC_EXPORTS size_t GetNumPlanes() const;

		/** Returns the plane at the specified number. */
		MOHPC_EXPORTS const Plane *GetPlane(size_t planeNum);

		/** Returns the number of side equations. */
		MOHPC_EXPORTS size_t GetNumSideEquations() const;

		/** Returns the side equation at the specified number. */
		MOHPC_EXPORTS const SideEquation *GetSideEquation(size_t equationNum);

		/** Returns the number of brush sides. */
		MOHPC_EXPORTS size_t GetNumBrushSides() const;

		/** Returns the brush side at the specified number. */
		MOHPC_EXPORTS const BrushSide *GetBrushSide(size_t brushSideNum);

		/** Returns the number of brushes. */
		MOHPC_EXPORTS size_t GetNumBrushes() const;

		/** Returns the brush at the specified number. */
		MOHPC_EXPORTS const Brush *GetBrush(size_t brushNum) const;

		/** Returns the number of submodels. */
		MOHPC_EXPORTS size_t GetNumSubmodels() const;

		/** Returns the leaf at the specified number. */
		MOHPC_EXPORTS const Leaf *GetLeaf(size_t leafNum) const;

		/** Returns the number of leafs. */
		MOHPC_EXPORTS size_t GetNumLeafs() const;

		/** Returns the node at the specified number. */
		MOHPC_EXPORTS const Node* GetNode(size_t nodeNum) const;

		/** Returns the number of nodes. */
		MOHPC_EXPORTS size_t GetNumNodes() const;

		/** Returns the leaf brush at the specified number. */
		MOHPC_EXPORTS uintptr_t GetLeafBrush(size_t leafBrushNum) const;

		/** Returns the number of leaf brushes. */
		MOHPC_EXPORTS size_t GetNumLeafBrushes() const;

		/** Returns the leaf surface at the specified number. */
		MOHPC_EXPORTS uintptr_t GetLeafSurface(size_t leafSurfNum) const;

		/** Returns the number of leaf surfaces. */
		MOHPC_EXPORTS size_t GetNumLeafSurfaces() const;

		/** Returns the submodel at the specified number. */
		MOHPC_EXPORTS const Model *GetSubmodel(size_t submodelNum) const;

		/** Returns the submodel by name (in the format '*x'). */
		MOHPC_EXPORTS const Model *GetSubmodel(const str& submodelName) const;

		/** Returns the number of sphere lights. */
		MOHPC_EXPORTS size_t GetNumLights() const;

		/** Returns the light at the specified number. */
		MOHPC_EXPORTS const SphereLight *GetLight(size_t lightNum) const;

		/** Returns the number of static models. */
		MOHPC_EXPORTS size_t GetNumStaticModels() const;

		/** Returns the static model at the specified number. */
		MOHPC_EXPORTS const StaticModel *GetStaticModel(size_t staticModelNum) const;

		/** Returns the number of terrain patches. */
		MOHPC_EXPORTS size_t GetNumTerrainPatches() const;

		/** Returns the terrain patch at the specified number. */
		MOHPC_EXPORTS const TerrainPatch *GetTerrainPatch(size_t terrainPatchNum) const;

		/** Returns the number of terrain surfaces. */
		MOHPC_EXPORTS size_t GetNumTerrainSurfaces() const;

		/** Returns the terrain surface at the specified number. */
		MOHPC_EXPORTS const Surface *GetTerrainSurface(size_t terrainSurfaceNum) const;

		/** Returns the number of entities in this level (static models are not counted). */
		MOHPC_EXPORTS size_t GetNumEntities() const;

		/** Returns the entity at the specified number. */
		MOHPC_EXPORTS const class LevelEntity *GetEntity(size_t entityNum) const;

		/** Returns the first entity found with the targetname. */
		MOHPC_EXPORTS const class LevelEntity* GetEntity(const str& targetName) const;

		/** Returns an array of entities with the specified targetname. */
		MOHPC_EXPORTS const Container<class LevelEntity *>* GetEntities(const str& targetName) const;

		/** Returns the number of grouped surfaces. */
		MOHPC_EXPORTS size_t GetNumSurfacesGroup() const;

		/** Returns the grouped surface at the specified number. */
		MOHPC_EXPORTS const SurfacesGroup *GetSurfacesGroup(size_t surfsGroupNum) const;

		/** Generate planes of collision from a terrain patch. */
		MOHPC_EXPORTS void GenerateTerrainCollide(const TerrainPatch* patch, TerrainCollide& collision);

		/** Return the leaf number of the point at the specified location. */
		MOHPC_EXPORTS uintptr_t PointLeafNum(const MOHPC::Vector p);

		/** Fill the specified collision world for tracing, etc... */
		MOHPC_EXPORTS void FillCollisionWorld(CollisionWorld& cm);

		static bool PlaneFromPoints(vec4_t plane, vec3_t a, vec3_t b, vec3_t c);

	protected:
		bool Load() override;

	private:
		Plane::PlaneType PlaneTypeForNormal(const Vector& Normal);
		uintptr_t PointLeafNum_r(const MOHPC::Vector p, intptr_t num);

		//void PreAllocateLevelData(const File_Header *Header);
		void LoadShaders(const GameLump* GameLump);
		void LoadLightmaps(const GameLump* GameLump);

		void CreateSurfaceGridMesh(int32_t width, int32_t height, BSP::Vertice *ctrl, int32_t numIndexes, int32_t *indexes, BSP::Surface* grid);
		void SubdividePatchToGrid(int32_t Width, int32_t Height, const Vertice* Points, Surface* Out);
		PatchCollide* GeneratePatchCollide(int32_t width, int32_t height, const Vertice *points, float subdivisions);
		void ParseMesh(const File_Surface* InSurface, const File_Vertice* InVertices, Surface* Out);
		void ParseFace(const File_Surface* InSurface, const File_Vertice* InVertices, const int32_t* InIndices, Surface* Out);
		void ParseTriSurf(const File_Surface* InSurface, const File_Vertice* InVertices, const int32_t* InIndices, Surface* Out);
		void LoadSurfaces(const GameLump* Surfaces, const GameLump* Vertices, const GameLump* Indices);

		void LoadPlanes(const GameLump* GameLump);
		void LoadSideEquations(const GameLump* GameLump);
		void LoadBrushSides(const GameLump* GameLump);
		void LoadBrushes(const GameLump* GameLump);
		void LoadLeafs(const GameLump* GameLump);
		void LoadLeafsOld(const GameLump* GameLump);
		void LoadLeafsBrushes(const GameLump* GameLump);
		void LoadLeafSurfaces(const GameLump* GameLump);
		void LoadNodes(const GameLump* GameLump);
		void LoadVisibility(const GameLump* GameLump);
		void LoadSubmodels(const GameLump* GameLump);
		void LoadEntityString(const GameLump* GameLump);
		void LoadSphereLights(const GameLump* GameLump);
		void LoadStaticModelDefs(const GameLump* GameLump);
		void LoadTerrain(const GameLump* GameLump);
		void LoadTerrainIndexes(const GameLump* GameLump);
		void FloodArea(size_t areaNum, uint32_t floodNum, uint32_t& floodValid);
		void FloodAreaConnections();

		// terrain
		terraInt TR_AllocateVert(TerrainPatch *patch);
		void TR_InterpolateVert(TerrainTri *pTri, TerrainVert *pVert);
		void TR_ReleaseVert(TerrainPatch *patch, terraInt iVert);
		terraInt TR_AllocateTri(TerrainPatch *patch, uint8_t byConstChecks = 4);
		void TR_FixTriHeight(TerrainTri* pTri);
		void TR_SetTriConstChecks(TerrainTri* pTri);
		void TR_ReleaseTri(TerrainPatch *patch, terraInt iTri);
		void TR_DemoteInAncestry(TerrainPatch *patch, terraInt iTri);
		void TR_TerrainHeapInit();
		void TR_SplitTri(terraInt iSplit, terraInt iNewPt, terraInt iLeft, terraInt iRight, terraInt iRightOfLeft, terraInt iLeftOfRight);
		void TR_ForceSplit(terraInt iTri);
		void TR_ForceMerge(terraInt iTri);
		int TR_TerraTriNeighbor(TerrainPatch *terraPatches, int iPatch, int dir);
		void TR_PreTessellateTerrain();
		bool TR_NeedSplitTri(TerrainTri *pTri);
		void TR_DoTriSplitting();
		void TR_DoGeomorphs();
		bool TR_MergeInternalAggressive();
		bool TR_MergeInternalCautious();
		void TR_DoTriMerging();
		void TR_ShrinkData();

		// terrain collision
		void TR_CalculateTerrainIndices(worknode_t* worknode, int iDiagonal, int iTree);
		void TR_PrepareGerrainCollide();
		void TR_PickTerrainSquareMode(TerrainCollideSquare* square, const MOHPC::Vector& vTest, terraInt i, terraInt j, const TerrainPatch* patch);

		void GenerateTerrainPatch(const TerrainPatch* Patch, Surface* Out);
		void GenerateTerrainPatch2(const TerrainPatch* Patch, Surface* Out);
		void UnpackTerraPatch(const File_TerrainPatch* Packed, TerrainPatch* Unpacked) const;

		void BoundBrush(Brush* Brush);
		void ColorShiftLightingFloats(float in[4], float out[4], float scale);
		void ColorShiftLightingFloats3(vec3_t in, vec3_t out, float scale);

		void CreateTerrainSurfaces();
		void CreateEntities();
		void MapBrushes();

		int32_t LoadLump(FilePtr file, File_Lump* Lump, GameLump* GameLump, size_t Size = 0);
		void FreeLump(GameLump *lump);

	private:
		Container<Shader> shaders;
		Container<Lightmap> lightmaps;
		Container<Surface> surfaces;
		Container<Plane> planes;
		Container<SideEquation> sideEquations;
		Container<BrushSide> brushSides;
		Container<Brush> brushes;
		Container<Node> nodes;
		Container<Leaf> leafs;
		Container<uintptr_t> leafBrushes;
		Container<uintptr_t> leafSurfaces;
		Container<TerrainPatch*> leafTerrains;
		Container<Area> areas;
		Container<uintptr_t> areaPortals;
		Container<Model> brushModels;
		Container<SphereLight> lights;
		Container<StaticModel> staticModels;
		Container<TerrainPatch> terrainPatches;
		Container<Surface> terrainSurfaces;
		Container<class LevelEntity*> entities;
		con_set<str, Container<class LevelEntity*>> targetList;
		Container<SurfacesGroup*> surfacesGroups;
		size_t numClusters;
		size_t numAreas;
		size_t clusterBytes;
		Container<uint8_t> visibility;
		char* entityString;
		size_t entityStringLength;

		Container<TerrainVert> trVerts;
		Container<TerrainTri> trTris;
		PoolInfo trpiTri;
		PoolInfo trpiVert;
		varnodeIndex varnodeIndexes[2][8][8][2];
	};
	using BSPPtr = SharedPtr<BSP>;
}
