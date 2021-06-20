#pragma once

#include "Collision.h"
#include "../Archive.h"

namespace MOHPC
{
	class CollisionWorldSerializer
	{
	public:
		CollisionWorldSerializer(CollisionWorld& cmValue)
			: cm(cmValue)
		{}

		MOHPC_UTILITY_EXPORTS void save(IArchiveWriter& ar);
		MOHPC_UTILITY_EXPORTS void load(IArchiveReader& ar);

	private:
		CollisionWorld& cm;
	};

	template<typename T>
	class CollisionSerializerBase
	{
	public:
		using DataType = T;

	public:
		CollisionSerializerBase(T& objValue)
			: obj(objValue)
		{}

	protected:
		T& obj;
	};

	class CollisionPlaneSerializer : public CollisionSerializerBase<collisionPlane_t>
	{
	public:
		using CollisionSerializerBase::CollisionSerializerBase;

	public:
		void serialize(IArchive& ar);
	};

	class CollisionNodeSerializer : public CollisionSerializerBase<collisionNode_t>
	{
	public:
		using CollisionSerializerBase::CollisionSerializerBase;

	public:
		void save(IArchiveWriter& ar, const collisionPlane_t* planeList);
		void load(IArchiveReader& ar, const collisionPlane_t* planeList);
	};

	class CollisionLeafSerializer : public CollisionSerializerBase<collisionLeaf_t>
	{
	public:
		using CollisionSerializerBase::CollisionSerializerBase;

	public:
		void serialize(IArchive& ar);
	};

	class CollisionMaskSerializer : public CollisionSerializerBase<collisionFencemask_t>
	{
	public:
		using CollisionSerializerBase::CollisionSerializerBase;

	public:
		void save(IArchiveWriter& ar);
		void load(IArchiveReader& ar);
	};

	class CollisionShaderSerializer : public CollisionSerializerBase<collisionShader_t>
	{
	public:
		using CollisionSerializerBase::CollisionSerializerBase;

	public:
		void save(IArchiveWriter& ar, const collisionFencemask_t* fencemaskList);
		void load(IArchiveReader& ar, collisionFencemask_t* fencemaskList);
	};

	class TerrainCollideSquareSerializer : public CollisionSerializerBase<terrainCollideSquare_t>
	{
	public:
		using CollisionSerializerBase::CollisionSerializerBase;

	public:
		void serialize(IArchive& ar);
	};

	class TerrainCollideSerializer : public CollisionSerializerBase<terrainCollide_t>
	{
	public:
		using CollisionSerializerBase::CollisionSerializerBase;

	public:
		void serialize(IArchive& ar);
	};

	class CollisionTerrainSerializer : public CollisionSerializerBase<collisionTerrain_t>
	{
	public:
		using CollisionSerializerBase::CollisionSerializerBase;

	public:
		void serialize(IArchive& ar);
	};

	class CollisionAreaSerializer : public CollisionSerializerBase<collisionArea_t>
	{
	public:
		using CollisionSerializerBase::CollisionSerializerBase;

	public:
		void serialize(IArchive& ar);
	};

	class CollisionModelSerializer : public CollisionSerializerBase<collisionModel_t>
	{
	public:
		using CollisionSerializerBase::CollisionSerializerBase;

	public:
		void serialize(IArchive& ar);
	};

	class CollisionSideEqSerializer : public CollisionSerializerBase<collisionSideEq_t>
	{
	public:
		using CollisionSerializerBase::CollisionSerializerBase;

	public:
		void serialize(IArchive& ar);
	};

	class CollisionBrushSideSerializer : public CollisionSerializerBase<collisionBrushSide_t>
	{
	public:
		using CollisionSerializerBase::CollisionSerializerBase;

	public:
		void save(IArchiveWriter& ar, collisionPlane_t* planeList, collisionSideEq_t* sideEqList);
		void load(IArchiveReader& ar, collisionPlane_t* planeList, collisionSideEq_t* sideEqList);
	};

	class CollisionBrushSerializer : public CollisionSerializerBase<collisionBrush_t>
	{
	public:
		using CollisionSerializerBase::CollisionSerializerBase;

	public:
		void serialize(IArchive& ar);
		void save(IArchiveWriter& ar, collisionBrushSide_t* brushSidesList);
		void load(IArchiveReader& ar, collisionBrushSide_t* brushSidesList);
	};

	class CollisionPatchPlaneSerializer : public CollisionSerializerBase<patchPlane_t>
	{
	public:
		using CollisionSerializerBase::CollisionSerializerBase;

	public:
		void serialize(IArchive& ar);
	};

	class CollisionFacetSerializer : public CollisionSerializerBase<facet_t>
	{
	public:
		using CollisionSerializerBase::CollisionSerializerBase;

	public:
		void serialize(IArchive& ar);
	};

	class CollisionPatchCollideSerializer : public CollisionSerializerBase<patchCollide_t>
	{
	public:
		using CollisionSerializerBase::CollisionSerializerBase;

	public:
		void serialize(IArchive& ar);
		void save(IArchiveWriter& ar);
		void load(IArchiveReader& ar);
	};

	class CollisionPatchSerializer : public CollisionSerializerBase<collisionPatch_t>
	{
	public:
		using CollisionSerializerBase::CollisionSerializerBase;

	public:
		void save(IArchiveWriter& ar);
		void load(IArchiveReader& ar);
	};
}