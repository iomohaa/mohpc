#include <Shared.h>
#include <MOHPC/Assets/Managers/SoundManager.h>
#include <MOHPC/Files/Managers/IFileManager.h>
#include <MOHPC/Files/Category.h>
#include <MOHPC/Utility/TokenParser.h>
#include <MOHPC/Common/Log.h>

#include <cstring>

#define MOHPC_LOG_NAMESPACE "soundman"

using namespace MOHPC;

static SoundChannel_e S_ChannelNameToNum(const char *pszName)
{
	if (!strHelpers::icmp(pszName, "auto"))
	{
		return CHAN_AUTO;
	}
	else if (!strHelpers::icmp(pszName, "local"))
	{
		return CHAN_LOCAL;
	}
	else if (!strHelpers::icmp(pszName, "weapon"))
	{
		return CHAN_WEAPON;
	}
	else if (!strHelpers::icmp(pszName, "voice"))
	{
		return CHAN_VOICE;
	}
	else if (!strHelpers::icmp(pszName, "item"))
	{
		return CHAN_ITEM;
	}
	else if (!strHelpers::icmp(pszName, "body"))
	{
		return CHAN_BODY;
	}
	else if (!strHelpers::icmp(pszName, "dialog"))
	{
		return CHAN_DIALOG;
	}
	else if (!strHelpers::icmp(pszName, "dialog_secondary"))
	{
		return CHAN_DIALOG_SECONDARY;
	}
	else if (!strHelpers::icmp(pszName, "weaponidle"))
	{
		return CHAN_WEAPONIDLE;
	}
	else if (!strHelpers::icmp(pszName, "menu"))
	{
		return CHAN_MENU;
	}
	else
	{
		return CHAN_NONE;
	}
}

size_t SoundHash::operator()(const str& Keyval) const
{
	size_t h = 0;

	for (size_t i = 0; i < Keyval.length(); ++i) {
		h = tolower(Keyval[i]) + h * 31;
	}

	return h;
}

bool SoundEqual::operator()(const str& Left, const str& Right) const
{
	return Left.length() == Right.length() ? !strHelpers::icmp(Left.c_str(), Right.c_str()) : false;
}

bool SoundLess::operator()(const SoundNode* Left, const SoundNode* Right) const
{
	return strHelpers::icmp(Left->GetAliasName(), Right->GetAliasName()) < 0;
}

SoundResults::SoundResults()
{
	numNodes = 0;
	totalNodesSize = 0;
}

SoundNode::SoundNode()
{
	aliasName = nullptr;
	realName = nullptr;
	subtitle = nullptr;
	bStreamed = false;
	channel = CHAN_NONE;
	next = nullptr;
	volume = 0.f;
	volumeMod = 0.f;
	pitch = 0.f;
	pitchMod = 0.f;
	dist = 0.f;
	maxDist = 0.f;
}

const char* SoundNode::GetAliasName() const
{
	//return (const char*)((const char*)this + aliasName);
	return aliasName;
}

const char* SoundNode::GetRealName() const
{
	//return (const char*)((const char*)this + realName);
	return realName;
}

const char* SoundNode::GetSubtitle() const
{
	//return (const char*)((const char*)this + subtitle);
	return subtitle;
}

const char* SoundNode::GetChannelName() const
{
	switch (channel)
	{
	case CHAN_AUTO:
		return "auto";
	case CHAN_LOCAL:
		return "local";
	case CHAN_WEAPON:
		return "weapon";
	case CHAN_VOICE:
		return "voice";
	case CHAN_ITEM:
		return "item";
	case CHAN_BODY:
		return "body";
	case CHAN_DIALOG:
		return "dialog";
	case CHAN_DIALOG_SECONDARY:
		return "dialog_secondary";
	case CHAN_WEAPONIDLE:
		return "weaponidle";
	case CHAN_MENU:
		return "menu";
	default:
		return "";
	}
}

str SoundNode::GetAliasNameNotRandom() const
{
	const char* name = GetAliasName();
	const char* aliasNameEnd = name + strHelpers::len(name) - 1;

	const char* p = aliasNameEnd;
	while (isdigit(*p) && p != name) p--;

	if (p != aliasNameEnd)
	{
		str newStr;
		newStr.assign(name, p - name + 1);
		return newStr;
	}
	else
	{
		return name;
	}
}

size_t SoundNode::GetAliasNameNotRandomLength() const
{
	const char* name = GetAliasName();
	const char* aliasNameEnd = name + strHelpers::len(name) - 1;

	const char* p = aliasNameEnd;
	while (isdigit(*p) && p != name) p--;

	return p - name + 1;
}

bool SoundNode::IsRandomized() const
{
	const char* name = GetAliasName();
	const char* p = name + strHelpers::len(name) - 1;

	if (isdigit(*p))
	{
		return true;
	}
	else
	{
		return false;
	}
}

MOHPC_OBJECT_DEFINITION(SoundManager);
SoundManager::SoundManager()
{
	rootSoundNode = nullptr;
	startSoundNode = nullptr;
	numNodes = 0;
}

SoundManager::~SoundManager()
{
	if (rootSoundNode)
	{
		delete[] (uint8_t*)rootSoundNode;
		rootSoundNode = nullptr;
	}
}

void SoundManager::Init()
{
	size_t totalSize = 0;
	SoundResults results;

	MOHPC_LOG(Info, "Loading sound aliases.");

	std::vector<const FileCategory*> categoryList;
	GetFileCategoryManager()->GetCategoryList(categoryList);

	numNodes = 0;
	
	const size_t numCategory = categoryList.size();
	for (size_t i = 0; i < numCategory; i++)
	{
		const FileCategory* category = categoryList[i];
		if (!shouldProcessCategory(category)) {
			continue;
		}

		results = ParseUbersound("ubersound/ubersound.scr", category, nullptr);
		numNodes += results.numNodes;
		totalSize += results.totalNodesSize;

		results = ParseUbersound("ubersound/uberdialog.scr", category, nullptr);
		numNodes += results.numNodes;
		totalSize += results.totalNodesSize;
	}

	if (numNodes)
	{
		soundNodeMap.reserve(numNodes);

		rootSoundNode = (SoundNode*)new uint8_t[totalSize];

		SoundNode* firstSoundNode = rootSoundNode;
		SoundNode* lastSoundNode = nullptr;

		for (size_t i = 0; i < numCategory; i++)
		{
			const FileCategory* category = categoryList[i];
			if (!shouldProcessCategory(category)) {
				continue;
			}

			results = ParseUbersound("ubersound/ubersound.scr", category, firstSoundNode, &lastSoundNode);
			firstSoundNode = (SoundNode*)((uint8_t*)firstSoundNode + results.totalNodesSize);

			results = ParseUbersound("ubersound/uberdialog.scr", category, firstSoundNode, &lastSoundNode);
			firstSoundNode = (SoundNode*)((uint8_t*)firstSoundNode + results.totalNodesSize);
		}

		SortList();
	}

	MOHPC_LOG(Info, "%zu sound alias(es).", numNodes);
}


const SoundNode* SoundManager::GetFirstAlias() const
{
	return startSoundNode;
}

size_t SoundManager::GetNumAliases() const
{
	return numNodes;
}

const SoundNode* SoundManager::FindAlias(const char* aliasName, bool bAllowRandom) const
{
	const SoundNode* soundNode = nullptr;

	auto it = soundNodeMap.find(aliasName);
	if (it != soundNodeMap.end())
	{
		soundNode = it->second;

		if (bAllowRandom)
		{
			const size_t aliasNameLen = soundNode->GetAliasNameNotRandomLength();

			size_t numRandom = 0;
			for (const SoundNode* randomNode = soundNode; randomNode; randomNode = randomNode->next)
			{
				if (strnicmp(randomNode->GetAliasName(), soundNode->GetAliasName(), aliasNameLen) || !randomNode->IsRandomized())
				{
					break;
				}

				numRandom++;
			}

			if (numRandom > 0)
			{
				const size_t nodeIndex = rand() % numRandom;

				size_t index = 0;
				for (const SoundNode* randomNode = soundNode; randomNode; randomNode = randomNode->next)
				{
					if (index == nodeIndex)
					{
						soundNode = randomNode;
						break;
					}

					index++;
				}
			}
		}
	}

	return soundNode;
}

bool SoundManager::shouldProcessCategory(const FileCategory* category) const
{
	// ignore children category, only process root
	return !category->getParent();
}

SoundResults SoundManager::ParseUbersound(const fs::path& filename, const FileCategory* category, SoundNode* firstSoundNode, SoundNode** ppLastSoundNode)
{
	TokenParser script;

	const char* const categoryName = category ? category->getName() : nullptr;

	IFilePtr File = GetFileManager()->OpenFile(filename, category);
	if (!File)
	{
		if (firstSoundNode) {
			MOHPC_LOG(Warn, "ubersound '%s' not found for %s.", filename.generic_string().c_str(), categoryName);
		}

		return SoundResults();
	}

	const char* buf = nullptr;
	uint64_t streamSize = File->ReadBuffer((void**)&buf);

	script.Parse(buf, streamSize);

	SoundResults results;

	if (firstSoundNode)
	{
		MOHPC_LOG(Warn, "parsing ubersound '%s' from %s", filename.generic_string().c_str(), categoryName);

		SoundNode* soundNode = firstSoundNode;
		SoundNode* prevSoundNode = ppLastSoundNode ? *ppLastSoundNode : nullptr;

		while (script.TokenAvailable(true))
		{
			const char* token = script.GetToken(true);
			if (!strHelpers::icmp(token, "aliascache") || !strHelpers::icmp(token, "alias"))
			{
				const size_t soundSize = ParseAlias(script, soundNode) + sizeof(SoundNode);
				if (soundSize)
				{
					if (prevSoundNode) {
						prevSoundNode->next = soundNode;
					}

					soundNodeMap.insert_or_assign(soundNode->GetAliasName(), soundNode);

					prevSoundNode = soundNode;
					soundNode = (SoundNode*)((uint8_t*)soundNode + soundSize);

					results.numNodes++;
					results.totalNodesSize += soundSize;
				}
			}
			else if(!strHelpers::icmp(token, "end"))
			{
				// end instruction reached
				break;
			}
			else if(!strHelpers::icmp(token, "settiki"))
			{
				token = script.GetToken(false);
				// ignore settiki command, not useful in the library
			}
			else
			{
				MOHPC_LOG(Warn, "unsupported command '%s' in ubersound '%s' from %s.", token, filename.generic_string().c_str(), categoryName);
				script.SkipToEOL();
			}
		}

		if (ppLastSoundNode)
		{
			*ppLastSoundNode = prevSoundNode;
		}
	}
	else
	{
		while (script.TokenAvailable(true))
		{
			const char* token = script.GetToken(true);
			if (!strHelpers::icmp(token, "aliascache") || !strHelpers::icmp(token, "alias"))
			{
				const size_t soundSize = ParseAlias(script, nullptr) + sizeof(SoundNode);
				if(soundSize)
				{
					results.numNodes++;
					results.totalNodesSize += soundSize;
				}
			}
			else if (!strHelpers::icmp(token, "end"))
			{
				// end instruction reached
				break;
			}
			else if (!strHelpers::icmp(token, "settiki"))
			{
				token = script.GetToken(false);
				// ignore settiki command, not useful in the library
			}
			else {
				script.SkipToEOL();
			}
		}
	}

	return results;
}

size_t SoundManager::ParseAlias(TokenParser& script, SoundNode* soundNode)
{
	const char* token = script.GetToken(true);
	const size_t aliasNameSize = strHelpers::len(token) + 1;

	if (soundNode)
	{
		*soundNode = SoundNode();

		char* p = (char*)soundNode + sizeof(SoundNode);
		memcpy(p, token, aliasNameSize);
		soundNode->aliasName = p;
	}

	token = script.GetToken(true);
	const size_t realNameSize = strHelpers::len(token) + 1;

	if (soundNode)
	{
		char* p = (char*)soundNode + sizeof(SoundNode) + aliasNameSize;
		memcpy(p, token, realNameSize);
		soundNode->realName = p;
	}

	token = script.GetToken(true);
	if (strHelpers::icmp(token, "soundparms"))
	{
		script.SkipToEOL();
		return 0;
	}

	const float volume = script.GetFloat(true);
	const float volumeMod = script.GetFloat(true);
	const float pitch = script.GetFloat(true);
	const float pitchMod = script.GetFloat(true);
	const float dist = script.GetFloat(true);
	const float maxDist = script.GetFloat(true);

	token = script.GetToken(true);
	SoundChannel_e channel = S_ChannelNameToNum(token);

	bool bStreamed = false;

	token = script.GetToken(true);
	if (!strHelpers::icmp(token, "streamed"))
	{
		bStreamed = true;
	}

	if (soundNode)
	{
		soundNode->volume = volume;
		soundNode->volumeMod = volumeMod;
		soundNode->pitch = pitch;
		soundNode->pitchMod = pitchMod;
		soundNode->dist = dist;
		soundNode->maxDist = maxDist;
		soundNode->channel = channel;
		soundNode->bStreamed = bStreamed;
	}

	size_t subtitleSize = 0;

	token = script.GetToken(true);
	if(!strHelpers::icmp(token, "always"))
	{
		// it is used in SH/BT to indicate that the sound will be always loaded
		token = script.GetToken(false);
	}

	if (!strHelpers::icmp(token, "subtitle") || !strHelpers::icmp(token, "forcesubtitle"))
	{
		token = script.GetString(true, true, true);
		subtitleSize = strHelpers::len(token) + 1;

		if (soundNode)
		{
			char* p = (char*)soundNode + sizeof(SoundNode) + aliasNameSize + realNameSize;
			memcpy(p, token, subtitleSize);
			soundNode->subtitle = p;
		}

		token = script.GetToken(true);
	}

	if(!strHelpers::icmp(token, "maps"))
	{
		token = script.GetString(true, true, true);
		// for now, ignore maps
	}

	return aliasNameSize + realNameSize + subtitleSize;
}

void SoundManager::SortList()
{
	std::set<SoundNode*, SoundLess> list;

	for (SoundNode* soundNode = rootSoundNode; soundNode != nullptr; soundNode = soundNode->next)
	{
		list.insert(soundNode);
	}
	
	if (list.size())
	{
		startSoundNode = *list.begin();

		for (std::set<SoundNode*, SoundLess>::iterator it = list.begin(); it != list.end(); ++it)
		{
			SoundNode* soundNode = *it;
			auto nextIt = it;
			nextIt++;

			const char* aliasName = soundNode->GetAliasName();
			const char* aliasNameEnd = aliasName + strHelpers::len(aliasName) - 1;

			const char* p = aliasNameEnd;
			while (isdigit(*p) && p != aliasName) p--;

			if (p != aliasNameEnd)
			{
				const str newAliasName(aliasName, p - aliasName + 1);

				if (soundNodeMap.find(newAliasName) == soundNodeMap.end())
				{
					soundNodeMap.insert(std::pair<str, SoundNode*>(newAliasName, soundNode));
				}
			}

			if (nextIt != list.end())
			{
				soundNode->next = *nextIt;
			}
			else
			{
				soundNode->next = nullptr;
			}
		}
	}
}
