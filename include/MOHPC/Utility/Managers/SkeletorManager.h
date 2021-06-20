#pragma once

#include "../../Assets/Managers/Manager.h"
#include "../UtilityGlobal.h"
#include "../UtilityObject.h"

namespace MOHPC
{
	class SkeletonChannelNameTable;
	class skelBone_World;

	class SkeletorManager : public Manager
	{
		MOHPC_UTILITY_OBJECT_DECLARATION(SkeletorManager);

	public:
		MOHPC_UTILITY_EXPORTS SkeletorManager();
		MOHPC_UTILITY_EXPORTS ~SkeletorManager();

		MOHPC_UTILITY_EXPORTS SkeletonChannelNameTable *GetBoneNamesTable() const;
		MOHPC_UTILITY_EXPORTS SkeletonChannelNameTable *GetChannelNamesTable() const;
		MOHPC_UTILITY_EXPORTS skelBone_World *GetWorldBone() const;

	private:
		SkeletonChannelNameTable* m_boneNames;
		SkeletonChannelNameTable* m_channelNames;
		skelBone_World* m_worldBone;
	};
	using SkeletorManagerPtr = SharedPtr<SkeletorManager>;
}
