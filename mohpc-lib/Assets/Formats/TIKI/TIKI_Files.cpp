#include <Shared.h>
#include <MOHPC/Assets/Formats/TIKI.h>
#include <MOHPC/Common/Log.h>
#include "TIKI_Private.h"
#include "../Skel/SkelPrivate.h"
#include "../../../Common/Memory/DummyAllocator.h"
#include "../../../Common/Memory/FixedAllocator.h"
#include <cstring>

#include "../../../Common/VectorPrivate.h"

static constexpr char MOHPC_LOG_NAMESPACE[] = "tikifiles";

using namespace MOHPC;

dloadanim_t::dloadanim_t()
{
	init();
}

dloadanim_t::dloadanim_t(const char* aliasValue)
	: alias(aliasValue)
{
	init();
}

void dloadanim_t::init()
{
	weight = 1.f;
	blendtime = 0.2f;
	flags = 0;
}

dloadanim_t::operator const char* () const
{
	return alias.c_str();
}

MOHPC_OBJECT_DEFINITION(TIKIReader);
TIKIReader::TIKIReader()
	: allowedIncludes(nullptr)
	, numAllowedIncludes(0)
{
}

TIKIReader::TIKIReader(const char* allowedIncludesPtr[], size_t numElements)
	: allowedIncludes(allowedIncludesPtr)
	, numAllowedIncludes(numElements)
{
}

TIKIReader::~TIKIReader()
{

}

AssetPtr TIKIReader::read(const IFilePtr& file)
{
	const fs::path& Fname = file->getName();

	TIKIPtr tiki;

	dloaddef_t loaddef;
	TIKIAnim* tikiAnim = LoadTIKIAnim(Fname, &loaddef);
	if (tikiAnim) {
		tiki = LoadTIKIModel(Fname, tikiAnim, &loaddef);
	}

	if (loaddef.tikiFile) {
		loaddef.tikiFile->SetCurrentScript(nullptr);
	}

	FreeStorage(&loaddef);

	return tiki;
}

bool TIKIReader::IsPredefinedFrame(frameInt_t frameNum) const
{
	if (frameNum == TF::LAST)
	{
		return true;
	}
	else if (frameNum == TF::END)
	{
		return true;
	}
	else if (frameNum == TF::ENTRY)
	{
		return true;
	}
	else if (frameNum == TF::EXIT)
	{
		return true;
	}
	else if (frameNum == TF::EVERY)
	{
		return true;
	}
	else if (frameNum == TF::FIRST)
	{
		return true;
	}

	return false;
}

bool TIKIReader::IsValidFrame(size_t maxFrames, frameInt_t frameNum) const
{
	if (IsPredefinedFrame(frameNum)) {
		return true;
	}

	return frameNum < maxFrames;
}

void TIKIReader::FixFrameNum(dloaddef_t* ld, const TIKIAnim *ptiki, const SkeletonAnimation *animData, TIKICommand *cmd, const char *alias)
{
	if (IsValidFrame(animData->GetNumFrames(), cmd->frame_num))
	{
		if (cmd->frame_num == TF::END || cmd->frame_num == TF::LAST)
		{
			// fix frame number when it is the end or the last
			cmd->frame_num = frameInt_t(animData->GetNumFrames() - 1);
		}
	}
	else
	{
		MOHPC_LOG(Error, "TIKI_FixFrameNum: illegal frame number %d (total: %d) in anim '%s' in '%s'", cmd->frame_num, animData->GetNumFrames(), alias, ld->tikiFile->getFilename().generic_string().c_str());
		cmd->frame_num = 0;
	}
}

void TIKIReader::LoadAnim(dloaddef_t* ld, const TIKIAnim* ptikianim)
{
	for (uintptr_t i = 0; i < ptikianim->num_animdefs; i++)
	{
		const TIKIAnimDef* const panimdef = &ptikianim->animdefs[i];
		const SkeletonAnimation* const animData = panimdef->animData.get();
		if (animData)
		{
			for (uintptr_t j = 0; j < panimdef->num_server_cmds; j++)
			{
				FixFrameNum(ld, ptikianim, animData, const_cast<TIKICommand*>(&panimdef->server_cmds[j]), panimdef->alias);
			}

			for (uintptr_t j = 0; j < panimdef->num_client_cmds; j++)
			{
				FixFrameNum(ld, ptikianim, animData, const_cast<TIKICommand*>(&panimdef->client_cmds[j]), panimdef->alias);
			}
		}
	}
}

TIKIAnim* TIKIReader::LoadTIKIAnim(const fs::path& Filename, dloaddef_t* ld)
{
	InitSetup(ld);

	ld->tikiFile = GetAssetManager()->readAsset<TikiScriptReader>(Filename);
	if (!ld->tikiFile) {
		throw AssetError::AssetNotFound(Filename);
	}

	const char* token = ld->tikiFile->GetToken(true);
	if (strHelpers::cmp(token, TIKI_HEADER))
	{
		MOHPC_LOG(Error, "TIKI_LoadTIKIfile: def file %s has wrong header (%s should be %s)", ld->tikiFile->getFilename().generic_string().c_str(), token, TIKI_HEADER);

		const uint8_t header[] = { uint8_t(token[0]),  uint8_t(token[1]),  uint8_t(token[2]),  uint8_t(token[3]) };
		throw AssetError::BadHeader4(header, (const uint8_t*)TIKI_HEADER);
	}

	ld->bInIncludesSection = false;
	ld->tikiFile->SetAllowExtendedComment(true);

	while (ld->tikiFile->TokenAvailable(true))
	{
		token = ld->tikiFile->GetToken(true);

		if (!strHelpers::icmp(token, "setup"))
		{
			if (!ParseSetup(ld))
			{
				// setup section not valid
				throw TIKIError::BadSetup();
			}
		}
		else if (!strHelpers::icmp(token, "init"))
		{
			ParseInit(ld);
		}
		else if (!strHelpers::icmp(token, "animations"))
		{
			ParseAnimations(ld);
		}
		else if (!strHelpers::icmp(token, "includes"))
		{
			if (!ld->bInIncludesSection)
			{
				ld->bInIncludesSection = ParseIncludes(ld);
			}
			else
			{
				MOHPC_LOG(Error, "TIKI_LoadTIKIfile: Nested Includes section in %s on line %d, the animations will be fubar",
					token,
					ld->tikiFile->GetLineNumber(),
					ld->tikiFile->getFilename().generic_string().c_str()
				);
			}
		}
		else if (!strHelpers::icmp(token, "}") && ld->bInIncludesSection)
		{
			ld->bInIncludesSection = false;
		}
		else if (!strHelpers::icmp(token, "/*QUAKED"))
		{
			ParseQuaked(ld);
		}
		else
		{
			MOHPC_LOG(Error, "TIKI_LoadTIKIfile: unknown section %s in %s online %d, skipping line.", token, ld->tikiFile->getFilename().generic_string().c_str(), ld->tikiFile->GetLineNumber());

			// skip the current line
			while (ld->tikiFile->TokenAvailable(false)) {
				ld->tikiFile->GetToken(false);
			}
		}
	}

	if (ld->bInIncludesSection)
	{
		// this shouldn't happen unless a closing brace is missing
		MOHPC_LOG(Error, "TIKI_LoadTIKIfile: Include section in %s did not terminate", ld->tikiFile->getFilename().generic_string().c_str());
	}

	TIKIAnim* TikiAnim = nullptr;

	if (ld->loadanims.size())
	{
		TikiAnim = InitTiki(ld);
		if (TikiAnim)
		{
			constexpr float maxLength = 100000.0f;
			constexpr float maxLengthSquared = maxLength * maxLength;

			const Vector3 tempVec = castVector(TikiAnim->maxs) - castVector(TikiAnim->mins);
			if (tempVec.squaredNorm() > maxLengthSquared)
			{
				castVector(TikiAnim->mins) = Vector3(-4.f, -4.f, -4.f);
				castVector(TikiAnim->maxs) = Vector3(4.f, 4.f, 4.f);
			}
		}
		else
		{
			MOHPC_LOG(Error, "TIKI_LoadTIKIfile: Failed to initialize animation %s.", ld->tikiFile->getFilename().generic_string().c_str());
			return nullptr;
		}
	}
	else
	{
		MOHPC_LOG(Error, "TIKI_LoadTIKIfile: No valid animations found in %s.", ld->tikiFile->getFilename().generic_string().c_str());
		return nullptr;
	}

	return TikiAnim;
}

TIKIPtr TIKIReader::LoadTIKIModel(const fs::path& Filename, TIKIAnim* tikiAnim, const dloaddef_t* ld)
{
	TIKIPtr tiki = TIKIPtr(new TIKI(Filename, getManager<SkeletorManager>(), tikiAnim));

	tiki_surface_list_t loadsurfaces;
	LoadSetup(*tiki, ld, loadsurfaces);

	std::vector<SkeletonPtr>& meshes = tiki->getMeshes();
	if (!meshes.size())
	{
		// meshes are not required, avoid throwing
		return nullptr;
	}

	SkeletonChannelList& boneList = tiki->getBoneList();
	boneList.InitChannels();
	boneList.ZeroChannels();

	for (size_t i = 0; i < meshes.size(); i++)
	{
		Skeleton *mesh = meshes[i].get();

		for (size_t j = 0; j < mesh->GetNumBones(); j++)
		{
			boneList.AddChannel(mesh->GetBone(j)->channel);
		}
	}

	boneList.PackChannels();

	for (auto it = loadsurfaces.begin(); it != loadsurfaces.end(); ++it)
	{
		const dloadsurface_t& loadsurf = *it;
		bool bFound = false;

		const char* strptr = strHelpers::find(loadsurf.name.c_str(), "*");

		size_t surfOffset = 0;

		std::vector<TIKISurface>& surfaces = tiki->getSurfaces();

		if (strptr || !strHelpers::icmp(loadsurf.name.c_str(), "all"))
		{
			for (size_t j = 0; j < meshes.size(); j++)
			{
				Skeleton *mesh = meshes[j].get();

				TIKISurface* tikiSurf = &surfaces[surfOffset];

				for (size_t k = 0; k < mesh->GetNumSurfaces(); k++)
				{
					const Surface* surf = mesh->GetSurface(k);

					if ((strptr
						&& strptr != loadsurf.name.c_str()
						&& !strHelpers::icmpn(loadsurf.name.c_str(), surf->name.c_str(), strptr - loadsurf.name.c_str()))
						|| !strHelpers::icmp(loadsurf.name.c_str(), "all"))
					{
						SetupIndividualSurface(tikiSurf, surf->name.c_str(), loadsurf);
						bFound = true;
					}

					tikiSurf++;
				}

				surfOffset += mesh->GetNumSurfaces();
			}
		}
		else
		{
			for (size_t j = 0; j < meshes.size(); j++)
			{
				Skeleton *mesh = meshes[j].get();

				TIKISurface* tikiSurf = &surfaces[surfOffset];

				for (size_t k = 0; k < mesh->GetNumSurfaces(); k++)
				{
					const Surface* surf = mesh->GetSurface(k);

					if (!strHelpers::icmp(loadsurf.name.c_str(), surf->name.c_str()))
					{
						SetupIndividualSurface(tikiSurf, surf->name.c_str(), loadsurf);
						if (!tikiSurf->name[0])
						{
							MOHPC_LOG(Warn, "TIKI_InitTiki: Surface %i in %s (referenced in %s) has no name!  Please investigate and fix", k, mesh->getFilename().generic_string().c_str(), Filename.generic_string().c_str());
						}
						bFound = true;
					}

					tikiSurf++;
				}

				surfOffset += mesh->GetNumSurfaces();
			}
		}

		if (!bFound)
		{
			MOHPC_LOG(Warn, "TIKI_InitTiki: could not find surface '%s' in '%s' (check referenced skb/skd files).", loadsurf.name.c_str(), tiki->getFilename().generic_string().c_str());
		}
	}

	return tiki;
}

void TIKIReader::FreeStorage(dloaddef_t* ld)
{
	if (ld->tikiFile)
	{
		//GetAssetManager()->UnloadAsset(ld->tikiFile);
		ld->tikiFile = nullptr;
	}
}

class AllocationCalculator
{
public:
	AllocationCalculator()
		: predefs(0)
	{}
	template<typename T>
	void alloc()
	{
		predefs += sizeof(T);
	}

	template<typename T>
	void alloc(size_t count)
	{
		predefs += sizeof(T) * count;
	}

	template<typename T>
	void allocPredef(size_t count)
	{
		predefs += sizeof(T) * count;
	}

	size_t calculateAllocationSize(const dloaddef_t* ld)
	{
		alloc<TIKIAnim>();

		// calculate the size of init commands
		calculateInitCommands(ld->loadserverinitcmds);
		calculateInitCommands(ld->loadclientinitcmds);
		calculateArray(ld->headmodels);
		calculateArray(ld->headskins);

		// calculate the size of animations
		calculateAnims(ld->loadanims);

		// calculate all strings
		calculateInitCommandArgs(ld->loadserverinitcmds);
		calculateInitCommandArgs(ld->loadclientinitcmds);

		calculateAnimsArgs(ld->loadanims);

		calculateArgs(ld->headmodels);
		calculateArgs(ld->headskins);

		return dummyAlloc.size() + predefs;
	}

	void calculateArray(const tiki_str_list_t& list)
	{
		allocPredef<char*>(list.size());
	}

	void calculateArgs(const tiki_str_list_t& list)
	{
		for (auto it = list.begin(); it != list.end(); ++it)
		{
			const size_t l = it->length();
			allocPredef<char>(l + 1);
		}
	}

	template<typename T>
	void calculateInitCommands(const T& list)
	{
		alloc<TIKICommand>(list.size());

		for (auto it = list.begin(); it != list.end(); ++it)
		{
			const typename T::value_type& ldcmd = *it;

			// calculate the init command size
			calculateArray(ldcmd.args);
		}
	}

	template<typename T>
	void calculateInitCommandArgs(const T& list)
	{
		for (auto it = list.begin(); it != list.end(); ++it)
		{
			const typename T::value_type& ldcmd = *it;

			// calculate the init command size
			calculateArgs(ldcmd.args);
		}
	}

	void calculateAnims(const tiki_anim_list_t& list)
	{
		alloc<TIKIAnimDef>(list.size());

		// 1st pass: fill animation structures
		// ...

		// 2nd pass: tiki commands
		for (auto it = list.begin(); it != list.end(); ++it)
		{
			const dloadanim_t& anim = *it;

			calculateInitCommands(anim.loadclientcmds);
			calculateInitCommands(anim.loadservercmds);
		}
	}

	void calculateAnimsArgs(const tiki_anim_list_t& list)
	{
		// 3rd pass: animation strings
		for (auto it = list.begin(); it != list.end(); ++it)
		{
			const dloadanim_t& anim = *it;

			allocPredef<char>(anim.alias.length() + 1);

			calculateInitCommandArgs(anim.loadclientcmds);
			calculateInitCommandArgs(anim.loadservercmds);
		}
	}

private:
	DummyAllocator dummyAlloc;
	size_t predefs = 0;
};

void copyArgs(FixedAllocator& allocator, const tiki_str_list_t& list, char** args, size_t& n)
{
	if (!list.size())
	{
		// no arguments
		n = 0;
		return;
	}

	for (auto it = list.begin(); it != list.end(); ++it)
	{
		const size_t l = it->length();
		char* arg = args[n++] = allocator.newObj<char>(l + 1);
		arg[l] = 0;
		std::copy(it->begin(), it->end(), arg);
	}
}

char** createArgs(FixedAllocator& allocator, const tiki_str_list_t& list)
{
	if (!list.size()) {
		return nullptr;
	}

	return allocator.newObj<char*>(list.size());
}

template<typename T>
TIKICommand* createInitCommands(FixedAllocator& allocator, const T& cmds)
{
	if (!cmds.size())
	{
		// nothing to process
		return nullptr;
	}

	TIKICommand* initcmds = allocator.newObj<TIKICommand>(cmds.size());
	size_t n = 0;
	for (auto it = cmds.begin(); it != cmds.end(); ++it)
	{
		const typename T::value_type& ldcmd = *it;
		TIKICommand& pcmd = initcmds[n++];

		pcmd.args = allocator.newObj<char*>(ldcmd.args.size());
	}

	return initcmds;
}

template<typename T>
void fillCommandsWithArgs(FixedAllocator& allocator, const T& cmds, TIKICommand* outcmds, size_t& n)
{
	if (!outcmds)
	{
		// nothing to process
		n = 0;
		return;
	}

	for (auto it = cmds.begin(); it != cmds.end(); ++it)
	{
		const typename T::value_type& ldcmd = *it;
		TIKICommand& pcmd = outcmds[n++];

		copyArgs(allocator, ldcmd.args, pcmd.args, pcmd.num_args);
	}
}

TIKIAnim * TIKIReader::InitTiki(dloaddef_t *ld)
{
	const size_t numAnims = ld->loadanims.size();
	if (!numAnims)
	{
		MOHPC_LOG(Error, "InitTiki(): no animation found in '%s', skipping", ld->tikiFile->getFilename().generic_string().c_str());
		return nullptr;
	}

	const size_t tikiSize = AllocationCalculator().calculateAllocationSize(ld);

	char* allocatedBuf = new char[tikiSize];
	FixedAllocator buf(allocatedBuf, tikiSize);
	const size_t startAlloc = buf.size();

	TIKIAnim* const panim = buf.newObj<TIKIAnim>();

	ClearBounds(panim->mins, panim->maxs);
	panim->bIsCharacter = ld->bIsCharacter;

	panim->server_initcmds = createInitCommands(buf, ld->loadserverinitcmds);
	panim->client_initcmds = createInitCommands(buf, ld->loadclientinitcmds);
	panim->headmodels = createArgs(buf, ld->headmodels);
	panim->headskins = createArgs(buf, ld->headskins);

	size_t numLoaded = 0;
	bool bModelBoundsSet = false;

	// Process anim commands
	panim->num_animdefs = ld->loadanims.size();
	panim->animdefs = buf.newObj<TIKIAnimDef>(panim->num_animdefs);
	size_t i = 0;
	for (auto it = ld->loadanims.begin(); it != ld->loadanims.end(); ++it, ++i)
	{
		const dloadanim_t& anim = *it;

		SkeletonAnimationPtr data;
		try
		{
			data = SkeletonAnimationReader::readNewAnim(GetAssetManager(), fs::path(anim.name));
			if (data.use_count() == 1)
			{
				// no one else is using sharing this anim
				numLoaded++;
			}
		}
		catch (const std::exception& exc)
		{
			MOHPC_LOG(Error, "InitTiki(): Error loading animation '%s': %s", anim.name.c_str(), exc.what());
			// continue loading anyway
		}

		TIKIAnimDef& animdef = panim->animdefs[i];
		animdef.weight = anim.weight;
		animdef.flags = anim.flags;
		animdef.animData = data;

		animdef.blendtime = anim.blendtime;

		animdef.server_cmds = createInitCommands(buf, anim.loadservercmds);
		animdef.client_cmds = createInitCommands(buf, anim.loadclientcmds);
	}

	// Process server init commands
	fillCommandsWithArgs(buf, ld->loadserverinitcmds, panim->server_initcmds, panim->num_server_initcmds);
	// Process client init commands
	fillCommandsWithArgs(buf, ld->loadclientinitcmds, panim->client_initcmds, panim->num_client_initcmds);

	i = 0;
	for (auto it = ld->loadanims.begin(); it != ld->loadanims.end(); ++it, ++i)
	{
		const dloadanim_t& anim = *it;
		TIKIAnimDef& animdef = panim->animdefs[i];

		const size_t aliasLen = anim.alias.length();
		animdef.alias = buf.newObj<char>(aliasLen + 1);
		animdef.alias[aliasLen] = 0;
		std::copy(anim.alias.begin(), anim.alias.end(), animdef.alias);

		if (!strHelpers::icmp(animdef.alias, "idle") && animdef.animData)
		{
			animdef.animData->GetBounds(panim->mins, panim->maxs);
			bModelBoundsSet = true;
		}

		if (anim.flags & TAF::RANDOM)
		{
			uintptr_t j = anim.alias.length();
			if (isdigit(animdef.alias[j - 1]))
			{
				do
				{
					j--;
				} while (isdigit(animdef.alias[j - 1]));

				animdef.alias[j] = 0;
			}
			else
			{
				MOHPC_LOG(Info, "TIKI_InitTiki: Random animation name '%s' should end with a number", animdef.alias);
			}
		}

		// Process server anim commands
		fillCommandsWithArgs(buf, anim.loadservercmds, animdef.server_cmds, animdef.num_server_cmds);
		// Process server anim commands
		fillCommandsWithArgs(buf, anim.loadclientcmds, animdef.client_cmds, animdef.num_client_cmds);
	}

	if (!bModelBoundsSet) {
		MOHPC_LOG(Warn, "TIKI_InitTiki: no 'idle' animation found, model bounds not set for %s", ld->tikiFile->getFilename().generic_string().c_str());
	}

	copyArgs(buf, ld->headmodels, panim->headmodels, panim->num_headmodels);
	copyArgs(buf, ld->headskins, panim->headskins, panim->num_headskins);

	LoadAnim(ld, panim);

	const size_t totalAllocated = buf.size() - startAlloc;
	assert(totalAllocated == tikiSize);

	MOHPC_LOG(Debug, "InitTiki(): %zu animations parsed, %zu animations valid and %zu animations loaded for '%s'.", numAnims, panim->num_animdefs, numLoaded, ld->tikiFile->getFilename().generic_string().c_str());

	return panim;
}

const char* TIKIError::BadSetup::what() const noexcept
{
	return "Invalid setup section";
}