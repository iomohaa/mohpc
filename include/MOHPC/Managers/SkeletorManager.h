#pragma once

#include "../Global.h"
#include "Manager.h"

namespace MOHPC
{
	class SkeletonChannelNameTable;
	class skelBone_World;

	class SkeletorManager : public Manager
	{
		CLASS_BODY(SkeletorManager);

	public:
		MOHPC_EXPORTS SkeletorManager();
		MOHPC_EXPORTS ~SkeletorManager();

		MOHPC_EXPORTS SkeletonChannelNameTable *GetBoneNamesTable() const;
		MOHPC_EXPORTS SkeletonChannelNameTable *GetChannelNamesTable() const;
		MOHPC_EXPORTS skelBone_World *GetWorldBone() const;

	private:
		SkeletonChannelNameTable* m_boneNames;
		SkeletonChannelNameTable* m_channelNames;
		skelBone_World* m_worldBone;
	};
	using SkeletorManagerPtr = SharedPtr<SkeletorManager>;
}
