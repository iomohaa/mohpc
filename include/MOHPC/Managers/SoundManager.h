#pragma once

#include "../Global.h"
#include "Manager.h"
#include <stdint.h>
#include <unordered_map>
#include <algorithm>
#include <ctype.h>

namespace MOHPC
{
	class Script;

	enum SoundChannel_e
	{
		CHAN_NONE = -1,
		CHAN_AUTO,
		CHAN_BODY,
		CHAN_ITEM,
		CHAN_WEAPONIDLE,
		CHAN_VOICE,
		CHAN_LOCAL,
		CHAN_WEAPON,
		CHAN_DIALOG_SECONDARY,
		CHAN_DIALOG,
		CHAN_MENU,
		CHAN_LOCAL_SOUND,
		CHAN_ANNOUNCER,
		CHAN_MAX
	};

	struct MOHPC_EXPORTS SoundNode
	{
		const char* aliasName;
		const char* realName;
		const char* subtitle;
		bool bStreamed;
		SoundChannel_e channel;

		// Static alias info
		SoundNode* next;

		// Global alias info
		float volume;
		float volumeMod;
		float pitch;
		float pitchMod;
		float dist;
		float maxDist;

		SoundNode();

		const char* GetAliasName() const;
		const char* GetRealName() const;
		const char* GetSubtitle() const;
		const char* GetChannelName() const;
		std::string GetAliasNameNotRandom() const;
		size_t GetAliasNameNotRandomLength() const;
		bool IsRandomized() const;
	};

	struct SoundHash
	{
		size_t operator()(const std::string& Keyval) const;
	};

	struct SoundEqual
	{
		bool operator()(const std::string& Left, const std::string& Right) const;
	};

	struct SoundLess
	{
		bool operator()(const SoundNode* Left, const SoundNode* Right) const;
	};

	struct MOHPC_EXPORTS SoundResults
	{
		size_t numNodes;
		size_t totalNodesSize;

		SoundResults();
	};

	/*
	 * Manager class used to manage ubersound/uberdialog
	 */
	class SoundManager : public Manager
	{
		CLASS_BODY(SoundManager);

	private:
		std::unordered_map<std::string, SoundNode*, SoundHash, SoundEqual> soundNodeMap;
		SoundNode* rootSoundNode;
		SoundNode* startSoundNode;
		size_t numNodes;

	public:
		MOHPC_EXPORTS SoundManager();
		MOHPC_EXPORTS ~SoundManager();

		/** Returns the first alias. */
		MOHPC_EXPORTS const SoundNode* GetFirstAlias() const;

		/** Returns the number of aliases*/
		MOHPC_EXPORTS size_t GetNumAliases() const;

		/** Finds an alias (random aliases are supported) */
		MOHPC_EXPORTS const SoundNode* FindAlias(const char* aliasName, bool bAllowRandom = true) const;

	protected:
		virtual void Init() override;

	private:
		/** Parse an ubersound and returns the total size of all sound nodes. */
		SoundResults ParseUbersound(const char* filename, const char* categoryName, SoundNode* firstSoundNode, SoundNode** ppLastSoundNode = nullptr);

		/** Parse an alias and returns the size of all strings in the sound node. */
		size_t ParseAlias(Script& script, SoundNode* soundNode);

		/** Sort the node list. */
		void SortList();
	};
}
