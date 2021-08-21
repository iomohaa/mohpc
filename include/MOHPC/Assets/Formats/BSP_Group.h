#pragma once

#include "../AssetsGlobal.h"
#include "../AssetBase.h"
#include "BSP.h"

namespace MOHPC
{
	namespace BSPData
	{
		struct BrushGroupData;

		class GroupedSurfaces
		{
		public:
			MOHPC_ASSETS_EXPORTS const char* GetGroupName() const;
			MOHPC_ASSETS_EXPORTS size_t GetNumSurfaces() const;
			MOHPC_ASSETS_EXPORTS const Surface* GetSurface(size_t index) const;
			MOHPC_ASSETS_EXPORTS size_t GetNumBrushesData() const;
			MOHPC_ASSETS_EXPORTS size_t GetBrushData(size_t index) const;
			MOHPC_ASSETS_EXPORTS const Surface* const* GetSurfaces() const;
			MOHPC_ASSETS_EXPORTS const_vec3p_t GetMinBound() const;
			MOHPC_ASSETS_EXPORTS const_vec3p_t GetMaxBound() const;
			MOHPC_ASSETS_EXPORTS const_vec3p_t GetOrigin() const;

		public:
			str name;
			std::vector<const Surface*> surfaces;
			std::vector<size_t> brushes;
			vec3_t bounds[2];
			vec3_t origin;
		};

		struct BrushGroupData
		{
		public:
			BrushGroupData();

			MOHPC_ASSETS_EXPORTS const Brush* getBrush() const;
			MOHPC_ASSETS_EXPORTS const BrushGroupData* getParent() const;
			MOHPC_ASSETS_EXPORTS size_t getNumSurfaces() const;
			MOHPC_ASSETS_EXPORTS const Surface* getSurface(size_t surfaceNum) const;

		public:
			const BSPData::Brush* brush;
			const BrushGroupData* parentData;
			std::vector<const Surface*> surfaces;
		};

		struct Patch;
	}

	class BSPGroup
	{
		MOHPC_ASSET_OBJECT_DECLARATION(BSPGroup);

	public:
		MOHPC_ASSETS_EXPORTS BSPGroup();
		MOHPC_ASSETS_EXPORTS ~BSPGroup();

		MOHPC_ASSETS_EXPORTS void groupSurfaces(const BSP& bsp);

		/** Returns the number of grouped surfaces. */
		MOHPC_ASSETS_EXPORTS size_t getNumGroupedSurfaces() const;

		/** Returns the grouped surface at the specified number. */
		MOHPC_ASSETS_EXPORTS const BSPData::GroupedSurfaces* getGroupedSurfaces(size_t surfsGroupNum) const;

		/** Returns the number of grouped surfaces. */
		MOHPC_ASSETS_EXPORTS size_t getNumBrushData() const;

		/** Returns the grouped surface at the specified number. */
		MOHPC_ASSETS_EXPORTS const BSPData::BrushGroupData* getBrushData(size_t brushDataNum) const;

	private:
		void connectBrushes(const BSP& bsp, std::vector<BSPData::BrushGroupData>& list);

		void mapBrushes(
			const BSP& bsp,
			std::vector<BSPData::GroupedSurfaces*>& surfacesGroups
		);

	private:
		std::vector<BSPData::GroupedSurfaces*> groupedSurfaces;
		std::vector<BSPData::BrushGroupData> brushDataList;
	};
	using BSPGroupPtr = SharedPtr<BSPGroup>;
}