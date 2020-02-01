#pragma once

#include "../Global.h"
#include "../Asset.h"
#include "../Managers/FileManager.h"
#include "../Managers/ShaderManager.h"
#include <stdint.h>
#include <vector>
#include <map>
#include <unordered_map>
#include "../Vector.h"
#include <string.h>

namespace MOHPC
{

	class File;
	class Shader;

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

	public:
		static const int32_t lightmapSize = 128;
		static const int32_t lightmapMemSize = lightmapSize * lightmapSize * sizeof(uint8_t) * 3;
		static const float lightmapTerrainLength;

		struct Shader
		{
			std::string shaderName;
			int32_t surfaceFlags;
			int32_t contentFlags;
			int32_t subdivisions;
			std::string fenceMaskImage;

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
			std::vector<Vertice> vertices;
			std::vector<int32_t> indexes;
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
			MOHPC_EXPORTS int32_t GetIndice(size_t index) const;

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
			std::string name;
			const Shader* shader;
			int32_t contents;
			Vector bounds[2];
			size_t numsides;
			BrushSide* sides;
			Brush* parent;
			std::vector<const Surface *> surfaces;

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

		class SurfacesGroup
		{
			friend BSP;

		private:
			std::string name;
			std::vector<const Surface *> surfaces;
			std::vector<const Brush *> brushes;
			Vector bounds[2];
			Vector origin;

		public:
			MOHPC_EXPORTS const std::string& GetGroupName() const;
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
			std::string modelName;
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

		/** Returns the submodel at the specified number. */
		MOHPC_EXPORTS const Model *GetSubmodel(size_t submodelNum) const;

		/** Returns the submodel by name (in the format '*x'). */
		MOHPC_EXPORTS const Model *GetSubmodel(const std::string& submodelName) const;

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
		MOHPC_EXPORTS const class LevelEntity* GetEntity(const std::string& targetName) const;

		/** Returns an array of entities with the specified targetname. */
		MOHPC_EXPORTS const std::vector<class LevelEntity *>* GetEntities(const std::string& targetName) const;

		/** Returns the number of grouped surfaces. */
		MOHPC_EXPORTS size_t GetNumSurfacesGroup() const;

		/** Returns the grouped surface at the specified number. */
		MOHPC_EXPORTS const SurfacesGroup *GetSurfacesGroup(size_t surfsGroupNum) const;

	protected:
		bool Load() override;

	private:
		Plane::PlaneType PlaneTypeForNormal(const Vector& Normal);

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
		void LoadSubmodels(const GameLump* GameLump);
		void LoadEntityString(const GameLump* GameLump);
		void LoadSphereLights(const GameLump* GameLump);
		void LoadStaticModelDefs(const GameLump* GameLump);
		void LoadTerrain(const GameLump* GameLump);

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
		std::vector<Shader> shaders;
		std::vector<Lightmap> lightmaps;
		std::vector<Surface> surfaces;
		std::vector<Plane> planes;
		std::vector<SideEquation> sideEquations;
		std::vector<BrushSide> brushSides;
		std::vector<Brush> brushes;
		std::vector<Model> brushModels;
		std::vector<SphereLight> lights;
		std::vector<StaticModel> staticModels;
		std::vector<TerrainPatch> terrainPatches;
		std::vector<Surface> terrainSurfaces;
		std::vector<class LevelEntity*> entities;
		std::unordered_map<std::string, std::vector<class LevelEntity*>> targetList;
		std::vector<SurfacesGroup*> surfacesGroups;
		char* entityString;
		size_t entityStringLength;

		std::vector<TerrainVert> trVerts;
		std::vector<TerrainTri> trTris;
		PoolInfo trpiTri;
		PoolInfo trpiVert;
	};
	typedef std::shared_ptr<BSP> BSPPtr;
}
