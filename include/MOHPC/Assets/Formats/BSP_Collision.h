#pragma once

#include "../AssetsGlobal.h"
#include "../AssetBase.h"
#include "../../Utility/SharedPtr.h"
#include "../../Common/SimpleVector.h"
#include "BSP_Terrain.h"

#include <cstdint>

namespace MOHPC
{
	namespace BSPData
	{
		struct TerrainPatch;
		struct PatchPlane;
		struct Facet;
		struct Vertice;

		struct PatchCollide
		{
		public:
			~PatchCollide();

		public:
			vec3_t bounds[2];
			int32_t numPlanes;
			PatchPlane* planes;
			int32_t numFacets;
			Facet* facets;
		};

		struct TerrainCollideSquare {
			vec4_t plane[2];
			int32_t eMode;
		};

		struct TerrainCollide {
			vec3_t vBounds[2];
			TerrainCollideSquare squares[8][8];
		};
	}

	class BSP;
	class CollisionWorld;
	struct worknode_t;

	class BSPCollision
	{
		MOHPC_ASSET_OBJECT_DECLARATION(BSPCollision);

	public:
		MOHPC_ASSETS_EXPORTS BSPCollision(const SharedPtr<BSP>& bspAsset);
		MOHPC_ASSETS_EXPORTS ~BSPCollision();

		/** Fill the specified collision world for tracing, etc... */
		MOHPC_ASSETS_EXPORTS void FillCollisionWorld(CollisionWorld& cm);

		/** Generate planes of collision from a terrain patch. */
		MOHPC_ASSETS_EXPORTS void GenerateTerrainCollide(const BSPData::TerrainPatch* patch, BSPData::TerrainCollide& collision);

	private:
		// terrain collision
		void TR_CalculateTerrainIndices(worknode_t* worknode, int iDiagonal, int iTree);
		void TR_PrepareGerrainCollide();
		void TR_PickTerrainSquareMode(BSPData::TerrainCollideSquare* square, const vec3r_t vTest, terraInt i, terraInt j, const BSPData::TerrainPatch* patch);

	private:
		SharedPtr<BSP> bsp;
		BSPData::varnodeIndex varnodeIndexes[2][8][8][2];
	};
	using BSPCollisionPtr = SharedPtr<BSPCollision>;
}