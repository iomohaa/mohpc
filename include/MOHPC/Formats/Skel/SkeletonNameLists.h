#pragma once

#include "../../Global.h"
#include <map>
#include <vector>
#include <stdint.h>

#define MAX_CHANNELS 200

namespace MOHPC
{
	struct SkeletonChannelName
	{
		std::string name;
		intptr_t channelNum;
	};

	class SkeletonChannelNameTable
	{
		std::vector<SkeletonChannelName> m_Channels;
		std::vector<intptr_t> m_lookup;

	public:
		SkeletonChannelNameTable();

		MOHPC_EXPORTS intptr_t RegisterChannel(const char* name);
		MOHPC_EXPORTS intptr_t FindNameLookup(const char* name);
		MOHPC_EXPORTS void PrintContents();
		MOHPC_EXPORTS const char* FindName(intptr_t index);
		MOHPC_EXPORTS intptr_t NumChannels() const;

		MOHPC_EXPORTS static bool IsBogusChannelName(const char *name);
		MOHPC_EXPORTS static int GetChannelTypeFromName(const char *name);

	private:
		const char *FindNameFromLookup(intptr_t index);
		bool FindIndexFromName(const char *name, intptr_t *indexPtr);
		void SortIntoTable(intptr_t index);
		void CopyChannel(SkeletonChannelName *dest, const SkeletonChannelName *source);
		void SetChannelName(SkeletonChannelName *channel, const char *newName);
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
		std::map<intptr_t, intptr_t> m_chanLocalFromGlobal;
		std::vector<intptr_t> m_chanGlobalFromLocal;

	public:
		MOHPC_EXPORTS intptr_t NumChannels(void) const;
		MOHPC_EXPORTS void ZeroChannels(void);
		MOHPC_EXPORTS void PackChannels(void);
		MOHPC_EXPORTS void InitChannels(void);
		MOHPC_EXPORTS void CleanUpChannels();
		MOHPC_EXPORTS intptr_t GlobalChannel(intptr_t localchannel) const;
		MOHPC_EXPORTS intptr_t LocalChannel(intptr_t channel) const;
		MOHPC_EXPORTS intptr_t GetGlobalFromLocal(intptr_t channel) const;
		MOHPC_EXPORTS intptr_t GetLocalFromGlobal(intptr_t globalChannel) const;
		MOHPC_EXPORTS void SetLocalFromGlobal(intptr_t channel, intptr_t localchannel);
		MOHPC_EXPORTS bool HasChannel(SkeletonChannelNameTable *nameTable, const char *channelName) const;
		MOHPC_EXPORTS bool HasChannel(intptr_t channel) const;
		MOHPC_EXPORTS intptr_t AddChannel(intptr_t newGlobalChannelNum);
		MOHPC_EXPORTS const char *ChannelName(SkeletonChannelNameTable *nameTable, intptr_t localChannelNum) const;
	};
};
