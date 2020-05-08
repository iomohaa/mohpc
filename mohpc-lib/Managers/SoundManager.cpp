#include <Shared.h>
#include <MOHPC/Managers/SoundManager.h>
#include <MOHPC/Managers/FileManager.h>
#include <MOHPC/Script.h>
#include <string.h>

using namespace MOHPC;

static SoundChannel_e S_ChannelNameToNum(const char *pszName)
{
	if (!stricmp(pszName, "auto"))
	{
		return CHAN_AUTO;
	}
	else if (!stricmp(pszName, "local"))
	{
		return CHAN_LOCAL;
	}
	else if (!stricmp(pszName, "weapon"))
	{
		return CHAN_WEAPON;
	}
	else if (!stricmp(pszName, "voice"))
	{
		return CHAN_VOICE;
	}
	else if (!stricmp(pszName, "item"))
	{
		return CHAN_ITEM;
	}
	else if (!stricmp(pszName, "body"))
	{
		return CHAN_BODY;
	}
	else if (!stricmp(pszName, "dialog"))
	{
		return CHAN_DIALOG;
	}
	else if (!stricmp(pszName, "dialog_secondary"))
	{
		return CHAN_DIALOG_SECONDARY;
	}
	else if (!stricmp(pszName, "weaponidle"))
	{
		return CHAN_WEAPONIDLE;
	}
	else if (!stricmp(pszName, "menu"))
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
	return Left.length() == Right.length() ? !stricmp(Left.c_str(), Right.c_str()) : false;
}

bool SoundLess::operator()(const SoundNode* Left, const SoundNode* Right) const
{
	return stricmp(Left->GetAliasName(), Right->GetAliasName()) < 0;
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
	const char* aliasNameEnd = name + strlen(name) - 1;

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
	const char* aliasNameEnd = name + strlen(name) - 1;

	const char* p = aliasNameEnd;
	while (isdigit(*p) && p != name) p--;

	return p - name + 1;
}

bool SoundNode::IsRandomized() const
{
	const char* name = GetAliasName();
	const char* p = name + strlen(name) - 1;

	if (isdigit(*p))
	{
		return true;
	}
	else
	{
		return false;
	}
}

CLASS_DEFINITION(SoundManager);
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

	Container<const char*> categoryList;
	GetFileManager()->GetCategoryList(categoryList);

	const size_t numCategory = categoryList.size();
	for (size_t i = 0; i < numCategory; i++)
	{
		results = ParseUbersound("ubersound/ubersound.scr", categoryList[i], nullptr);
		numNodes += results.numNodes;
		totalSize += results.totalNodesSize;

		results = ParseUbersound("ubersound/uberdialog.scr", categoryList[i], nullptr);
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
			results = ParseUbersound("ubersound/ubersound.scr", categoryList[i], firstSoundNode, &lastSoundNode);
			firstSoundNode = (SoundNode*)((uint8_t*)firstSoundNode + results.totalNodesSize);

			results = ParseUbersound("ubersound/uberdialog.scr", categoryList[i], firstSoundNode, &lastSoundNode);
			firstSoundNode = (SoundNode*)((uint8_t*)firstSoundNode + results.totalNodesSize);
		}

		SortList();
	}
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

SoundResults SoundManager::ParseUbersound(const char* filename, const char* categoryName, SoundNode* firstSoundNode, SoundNode** ppLastSoundNode)
{
	Script script;
	script.InitAssetManager(GetAssetManager());

	FilePtr File = GetFileManager()->OpenFile(filename, categoryName);
	if (!File)
	{
		return SoundResults();
	}

	const char* buf = nullptr;
	std::streamsize streamSize = File->ReadBuffer((void**)&buf);

	script.LoadFile(filename, (int)streamSize, buf);

	SoundResults results;

	if (firstSoundNode)
	{
		SoundNode* soundNode = firstSoundNode;
		SoundNode* prevSoundNode = ppLastSoundNode ? *ppLastSoundNode : nullptr;

		while (script.TokenAvailable(true))
		{
			const char* token = script.GetToken(true);
			if (!stricmp(token, "aliascache") || !stricmp(token, "alias"))
			{
				const size_t soundSize = ParseAlias(script, soundNode) + sizeof(SoundNode);
				if (soundSize)
				{
					if (prevSoundNode)
					{
						prevSoundNode->next = soundNode;
					}

					soundNodeMap.insert_or_assign(soundNode->GetAliasName(), soundNode);

					prevSoundNode = soundNode;
					soundNode = (SoundNode*)((uint8_t*)soundNode + soundSize);

					results.numNodes++;
					results.totalNodesSize += soundSize;
				}
			}
			else
			{
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
			if (!stricmp(token, "aliascache") || !stricmp(token, "alias"))
			{
				const size_t soundSize = ParseAlias(script, nullptr) + sizeof(SoundNode);
				if(soundSize)
				{
					results.numNodes++;
					results.totalNodesSize += soundSize;
				}
			}
			else
			{
				script.SkipToEOL();
			}
		}
	}

	return results;
}

size_t SoundManager::ParseAlias(Script& script, SoundNode* soundNode)
{
	const char* token = script.GetToken(false);
	const size_t aliasNameSize = strlen(token) + 1;

	if (soundNode)
	{
		*soundNode = SoundNode();

		char* p = (char*)soundNode + sizeof(SoundNode);
		memcpy(p, token, aliasNameSize);
		soundNode->aliasName = p;
	}

	token = script.GetToken(false);
	const size_t realNameSize = strlen(token) + 1;

	if (soundNode)
	{
		char* p = (char*)soundNode + sizeof(SoundNode) + aliasNameSize;
		memcpy(p, token, realNameSize);
		soundNode->realName = p;
	}

	token = script.GetToken(false);
	if (stricmp(token, "soundparms"))
	{
		script.SkipToEOL();
		return 0;
	}

	const float volume = script.GetFloat(false);
	const float volumeMod = script.GetFloat(false);
	const float pitch = script.GetFloat(false);
	const float pitchMod = script.GetFloat(false);
	const float dist = script.GetFloat(false);
	const float maxDist = script.GetFloat(false);

	token = script.GetToken(false);
	SoundChannel_e channel = S_ChannelNameToNum(token);

	bool bStreamed = false;

	token = script.GetToken(false);
	if (!stricmp(token, "streamed"))
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

	token = script.GetToken(false);
	if (!stricmp(token, "subtitle"))
	{
		token = script.GetToken(false);
		subtitleSize = strlen(token) + 1;

		if (soundNode)
		{
			char* p = (char*)soundNode + sizeof(SoundNode) + aliasNameSize + realNameSize;
			memcpy(p, token, subtitleSize);
			soundNode->subtitle = p;
		}
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
			const char* aliasNameEnd = aliasName + strlen(aliasName) - 1;

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
