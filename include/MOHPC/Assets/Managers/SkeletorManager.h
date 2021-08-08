#pragma once

#include "../../Assets/Managers/Manager.h"
#include "../AssetsGlobal.h"
#include "../AssetBase.h"

namespace MOHPC
{
	class SkeletonChannelNameTable;
	class skelBone_World;

	class SkeletorManager : public Manager
	{
		MOHPC_ASSET_OBJECT_DECLARATION(SkeletorManager);

	public:
		MOHPC_ASSETS_EXPORTS SkeletorManager();
		MOHPC_ASSETS_EXPORTS ~SkeletorManager();

		MOHPC_ASSETS_EXPORTS SkeletonChannelNameTable *GetBoneNamesTable() const;
		MOHPC_ASSETS_EXPORTS SkeletonChannelNameTable *GetChannelNamesTable() const;
		MOHPC_ASSETS_EXPORTS skelBone_World *GetWorldBone() const;

	private:
		SkeletonChannelNameTable* m_boneNames;
		SkeletonChannelNameTable* m_channelNames;
		skelBone_World* m_worldBone;
	};
	using SkeletorManagerPtr = SharedPtr<SkeletorManager>;
}
