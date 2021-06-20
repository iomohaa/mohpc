#pragma once

#include "../../AssetsGlobal.h"
#include "../../../Common/str.h"

#include <morfuse/Container/Container.h>
#include <morfuse/Container/set.h>
#include <cstdint>

namespace MOHPC
{
	static constexpr size_t MAX_CHANNELS = 20;
	static constexpr size_t MAX_CHANNEL_NAME = 32;

	struct SkeletonChannelName
	{
		str name;
		intptr_t channelNum;
	};

	class SkeletonChannelNameTable
	{
		mfuse::con::Container<SkeletonChannelName> m_Channels;
		mfuse::con::Container<intptr_t> m_lookup;

	public:
		SkeletonChannelNameTable();

		MOHPC_ASSETS_EXPORTS intptr_t RegisterChannel(const char* name);
		MOHPC_ASSETS_EXPORTS intptr_t FindNameLookup(const char* name) const;
		MOHPC_ASSETS_EXPORTS void PrintContents() const;
		MOHPC_ASSETS_EXPORTS const char* FindName(intptr_t index) const;
		MOHPC_ASSETS_EXPORTS intptr_t NumChannels() const;

		MOHPC_ASSETS_EXPORTS static bool IsBogusChannelName(const char *name);
		MOHPC_ASSETS_EXPORTS static int GetChannelTypeFromName(const char *name);

	private:
		const char *FindNameFromLookup(intptr_t index) const;
		bool FindIndexFromName(const char *name, intptr_t *indexPtr) const;
		void SortIntoTable(intptr_t index);
		void CopyChannel(SkeletonChannelName *dest, const SkeletonChannelName *source) const;
		void SetChannelName(SkeletonChannelName *channel, const char *newName) const;
	};

	class SkeletonChannelList
	{
	/*
	public:
		short int m_numChannels;

	private:
		short int m_numLocalFromGlobal;
		short int *m_chanLocalFromGlobal;

	public:
		short int m_chanGlobalFromLocal[MAX_CHANNELS];
	*/
	private:
		mfuse::con::set<intptr_t, intptr_t> m_chanLocalFromGlobal;
		mfuse::con::Container<intptr_t> m_chanGlobalFromLocal;

	public:
		MOHPC_ASSETS_EXPORTS intptr_t NumChannels(void) const;
		MOHPC_ASSETS_EXPORTS void ZeroChannels(void);
		MOHPC_ASSETS_EXPORTS void PackChannels(void);
		MOHPC_ASSETS_EXPORTS void InitChannels(void);
		MOHPC_ASSETS_EXPORTS void CleanUpChannels();
		MOHPC_ASSETS_EXPORTS intptr_t GlobalChannel(intptr_t localchannel) const;
		MOHPC_ASSETS_EXPORTS intptr_t LocalChannel(intptr_t channel) const;
		MOHPC_ASSETS_EXPORTS intptr_t GetGlobalFromLocal(intptr_t channel) const;
		MOHPC_ASSETS_EXPORTS intptr_t GetLocalFromGlobal(intptr_t globalChannel) const;
		MOHPC_ASSETS_EXPORTS void SetLocalFromGlobal(intptr_t channel, intptr_t localchannel);
		MOHPC_ASSETS_EXPORTS bool HasChannel(const SkeletonChannelNameTable *nameTable, const char *channelName) const;
		MOHPC_ASSETS_EXPORTS bool HasChannel(intptr_t channel) const;
		MOHPC_ASSETS_EXPORTS intptr_t AddChannel(intptr_t newGlobalChannelNum);
		MOHPC_ASSETS_EXPORTS const char *ChannelName(const SkeletonChannelNameTable *nameTable, intptr_t localChannelNum) const;
	};
};
