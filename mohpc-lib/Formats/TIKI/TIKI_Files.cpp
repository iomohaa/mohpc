#include <Shared.h>
#include <MOHPC/Formats/TIKI.h>
#include <MOHPC/Log.h>
#include "TIKI_Private.h"
#include "../Skel/SkelPrivate.h"
#include <cstring>

static constexpr char MOHPC_LOG_NAMESPACE[] = "tikifiles";

using namespace MOHPC;

void TIKI::Load()
{
	if (tikianim)
	{
		delete tikianim;
		tikianim = nullptr;
	}

	const char *Fname = GetFilename().c_str();

	dloaddef_t loaddef;
	if (LoadTIKIAnim(Fname, &loaddef)) {
		LoadTIKIModel(Fname, &loaddef);
	}

	if (loaddef.tikiFile)
	{
		HashCopy(loaddef.tikiFile.get());
		loaddef.tikiFile->SetCurrentScript(nullptr);
	}

	FreeStorage(&loaddef);
}

void TIKI::FixFrameNum(const TIKIAnim *ptiki, const SkeletonAnimation *animData, TIKIAnim::Command *cmd, const char *alias)
{
	if (cmd->frame_num >= TIKI_FRAME_LAST && cmd->frame_num < (intptr_t)animData->GetNumFrames())
	{
		if (cmd->frame_num <= TIKI_FRAME_END)
		{
			cmd->frame_num = animData->GetNumFrames() - 1;
		}
	}
	else
	{
		MOHPC_LOG(Error, "TIKI_FixFrameNum: illegal frame number %d (total: %d) in anim '%s' in '%s'", cmd->frame_num, animData->GetNumFrames(), alias, ptiki->name);
		cmd->frame_num = 0;
	}
}

void TIKI::LoadAnim(const TIKIAnim* ptikianim)
{
	for (uintptr_t i = 0; i < ptikianim->animdefs.size(); i++)
	{
		const TIKIAnim::AnimDef* const panimdef = &ptikianim->animdefs[i];
		const SkeletonAnimation* const animData = panimdef->animData.get();
		if (animData)
		{
			for (uintptr_t j = 0; j < panimdef->server_cmds.size(); j++)
			{
				FixFrameNum(ptikianim, animData, &panimdef->server_cmds[j], panimdef->alias.c_str());
			}

			for (uintptr_t j = 0; j < panimdef->client_cmds.size(); j++)
			{
				FixFrameNum(ptikianim, animData, &panimdef->client_cmds[j], panimdef->alias.c_str());
			}
		}
	}
}

bool MOHPC::TIKI::LoadTIKIAnim(const char* Filename, dloaddef_t* ld)
{
	InitSetup(ld);

	ld->tikiFile = GetAssetManager()->LoadAsset<TikiScript>(Filename);
	if (!ld->tikiFile) {
		throw AssetError::AssetNotFound(Filename);
	}

	ld->path = Filename;

	const char* token = ld->tikiFile->GetToken(true);
	if (str::cmp(token, TIKI_HEADER))
	{
		MOHPC_LOG(Error, "TIKI_LoadTIKIfile: def file %s has wrong header (%s should be %s)", ld->tikiFile->Filename(), token, TIKI_HEADER);

		const uint8_t header[] = { uint8_t(token[0]),  uint8_t(token[1]),  uint8_t(token[2]),  uint8_t(token[3]) };
		throw AssetError::BadHeader4(header, (const uint8_t*)TIKI_HEADER);
	}

	ld->bInIncludesSection = false;
	ld->tikiFile->SetAllowExtendedComment(true);

	while (ld->tikiFile->TokenAvailable(true))
	{
		token = ld->tikiFile->GetToken(true);

		if (!str::icmp(token, "setup"))
		{
			if (!ParseSetup(ld))
			{
				// setup section not valid
				throw TIKIError::BadSetup();
			}
		}
		else if (!str::icmp(token, "init"))
		{
			ParseInit(ld);
		}
		else if (!str::icmp(token, "animations"))
		{
			ParseAnimations(ld);
		}
		else if (!str::icmp(token, "includes"))
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
					ld->tikiFile->Filename()
				);
			}
		}
		else if (!str::icmp(token, "}") && ld->bInIncludesSection)
		{
			ld->bInIncludesSection = false;
		}
		else if (!str::icmp(token, "/*QUAKED"))
		{
			ParseQuaked(ld);
		}
		else
		{
			MOHPC_LOG(Error, "TIKI_LoadTIKIfile: unknown section %s in %s online %d, skipping line.", token, ld->tikiFile->Filename(), ld->tikiFile->GetLineNumber());

			// skip the current line
			while (ld->tikiFile->TokenAvailable(false)) {
				ld->tikiFile->GetToken(false);
			}
		}
	}

	if (ld->bInIncludesSection)
	{
		// this shouldn't happen unless a closing brace is missing
		MOHPC_LOG(Error, "TIKI_LoadTIKIfile: Include section in %s did not terminate", ld->tikiFile->Filename());
	}

	if (ld->loadanims.size())
	{
		TIKIAnim* const TikiAnim = InitTiki(ld);
		if (TikiAnim)
		{
			const Vector tempVec = TikiAnim->maxs - TikiAnim->mins;
			if (tempVec.length() > 100000.0f)
			{
				TikiAnim->mins = Vector(-4.f, -4.f, -4.f);
				TikiAnim->maxs = Vector(4.f, 4.f, 4.f);
			}

			tikianim = TikiAnim;
		}
		else
		{
			MOHPC_LOG(Error, "TIKI_LoadTIKIfile: Failed to initialize animation %s.", ld->tikiFile->Filename());
			return false;
		}
	}
	else
	{
		MOHPC_LOG(Error, "TIKI_LoadTIKIfile: No valid animations found in %s.", ld->tikiFile->Filename());
		return false;
	}

	return true;
}

bool TIKI::LoadTIKIModel(const char* Filename, const dloaddef_t* ld)
{
	Container<dloadsurface_t> loadsurfaces;
	LoadSetup(Filename, ld, loadsurfaces);

	if (!meshes.size())
	{
		// meshes are not required, avoid throwing
		return false;
	}

	name = Filename;
	boneList.InitChannels();
	boneList.ZeroChannels();

	for (size_t i = 0; i < meshes.size(); i++)
	{
		Skeleton *mesh = meshes[i].get();

		for (size_t j = 0; j < mesh->Bones.size(); j++)
		{
			boneList.AddChannel(mesh->Bones[j].channel);
		}
	}

	boneList.PackChannels();

	for (size_t i = 0; i < loadsurfaces.size(); i++)
	{
		dloadsurface_t* loadsurf = &loadsurfaces[i];
		bool bFound = false;

		const char* strptr = str::findchar(loadsurf->name.c_str(), '*');

		size_t surfOffset = 0;

		if (strptr || !str::icmp(loadsurf->name.c_str(), "all"))
		{
			for (size_t j = 0; j < meshes.size(); j++)
			{
				Skeleton *mesh = meshes[j].get();

				TIKISurface* tikiSurf = &surfaces[surfOffset];

				for (size_t k = 0; k < mesh->Surfaces.size(); k++)
				{
					Skeleton::Surface* surf = &mesh->Surfaces[k];

					if ((strptr
						&& strptr != loadsurf->name.c_str()
						&& !str::icmpn(loadsurf->name.c_str(), surf->name.c_str(), strptr - loadsurf->name.c_str()))
						|| !str::icmp(loadsurf->name.c_str(), "all"))
					{
						SetupIndividualSurface(tikianim->name.c_str(), tikiSurf, surf->name.c_str(), loadsurf);
						bFound = true;
					}

					tikiSurf++;
				}

				surfOffset += mesh->Surfaces.size();
			}
		}
		else
		{
			for (size_t j = 0; j < meshes.size(); j++)
			{
				Skeleton *mesh = meshes[j].get();

				TIKISurface* tikiSurf = &surfaces[surfOffset];

				for (size_t k = 0; k < mesh->Surfaces.size(); k++)
				{
					Skeleton::Surface* surf = &mesh->Surfaces[k];

					if (!str::icmp(loadsurf->name.c_str(), surf->name.c_str()))
					{
						SetupIndividualSurface(tikianim->name.c_str(), tikiSurf, surf->name.c_str(), loadsurf);
						if (!tikiSurf->name[0])
						{
							MOHPC_LOG(Warn, "TIKI_InitTiki: Surface %i in %s(referenced in %s) has no name!  Please investigate and fix", k, mesh->GetName(), name);
						}
						bFound = true;
					}

					tikiSurf++;
				}

				surfOffset += mesh->Surfaces.size();
			}
		}

		if (!bFound)
		{
			MOHPC_LOG(Warn, "TIKI_InitTiki: could not find surface '%s' in '%s' (check referenced skb/skd files).", loadsurf, tikianim->name);
		}
	}

	return true;
}

void TIKI::FreeStorage(dloaddef_t* ld)
{
	if (ld->tikiFile)
	{
		//GetAssetManager()->UnloadAsset(ld->tikiFile);
		ld->tikiFile = nullptr;
	}
}

TIKIAnim *TIKI::InitTiki(dloaddef_t *ld)
{
	TIKIAnim* const panim = new TIKIAnim;
	ClearBounds(panim->mins, panim->maxs);
	panim->bIsCharacter = ld->bIsCharacter;
	panim->name = ld->path;

	panim->server_initcmds.resize(ld->loadserverinitcmds.size());

	// Process server init commands
	for (uintptr_t i = 0; i < ld->loadserverinitcmds.size(); i++)
	{
		TIKIAnim::Command* const pcmds = &panim->server_initcmds[i];

		for (uintptr_t j = 0; j < ld->loadserverinitcmds[i].args.size(); j++)
		{
			pcmds->args.push_back(ld->loadserverinitcmds[i].args[j]);
		}
	}

	panim->client_initcmds.resize(ld->loadclientinitcmds.size());

	// Process client init commands
	for (uintptr_t i = 0; i < ld->loadclientinitcmds.size(); i++)
	{
		TIKIAnim::Command* const pcmds = &panim->client_initcmds[i];

		for (uintptr_t j = 0; j < ld->loadclientinitcmds[i].args.size(); j++)
		{
			pcmds->args.push_back(ld->loadclientinitcmds[i].args[j]);
		}
	}

	Container<size_t> order;
	GetAnimOrder(ld, order);

	const size_t numAnims = ld->loadanims.size();
	bool bModelBoundsSet = false;

	// Process anim commands
	for (uintptr_t i = 0; i < numAnims; i++)
	{
		const dloadanim_t* const anim = &ld->loadanims[order[i]];

		SkeletonAnimationPtr data;
		try
		{
			data = GetAssetManager()->LoadAsset<SkeletonAnimation>(anim->name.c_str());
		}
		catch (const std::exception& exc)
		{
			MOHPC_LOG(Error, "TIKI_InitTiki: Error loading animation '%s': %s", anim->name, exc.what());
		}

		if (!data)
		{
			MOHPC_LOG(Error, "TIKI_InitTiki: Failed to load animation '%s' at %s", anim->name, anim->location);
			continue;
		}

		TIKIAnim::AnimDef animdef;
		animdef.alias = anim->alias;
		animdef.weight = anim->weight;
		animdef.flags = anim->flags;
		animdef.animData = data;

		if (!str::icmp(animdef.alias.c_str(), "idle"))
		{
			data->GetBounds(panim->mins, panim->maxs);
			bModelBoundsSet = true;
		}


		if (anim->flags & TAF_RANDOM)
		{
			uintptr_t j = animdef.alias.length();
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

		animdef.blendtime = anim->blendtime;

		animdef.server_cmds.resize(anim->loadservercmds.size());

		// Process server anim commands
		for (uintptr_t j = 0; j < anim->loadservercmds.size(); j++)
		{
			TIKIAnim::Command* const pcmds = &animdef.server_cmds[j];
			pcmds->frame_num = anim->loadservercmds[j].frame_num;

			for (uintptr_t k = 0; k < anim->loadservercmds[j].args.size(); k++)
			{
				pcmds->args.push_back(anim->loadservercmds[j].args[k]);
			}
		}

		animdef.client_cmds.resize(anim->loadclientcmds.size());

		// Process server anim commands
		for (uintptr_t j = 0; j < anim->loadclientcmds.size(); j++)
		{
			TIKIAnim::Command* const pcmds = &animdef.client_cmds[j];
			pcmds->frame_num = anim->loadclientcmds[j].frame_num;

			for (uintptr_t k = 0; k < anim->loadclientcmds[j].args.size(); k++)
			{
				pcmds->args.push_back(anim->loadclientcmds[j].args[k]);
			}
		}

		panim->animdefs.push_back(animdef);
	}

	if (numAnims)
	{
		if (!bModelBoundsSet)
		{
			MOHPC_LOG(Warn, "TIKI_InitTiki: no 'idle' animation found, model bounds not set for %s", ld->path);
		}

		panim->headmodels = ld->headmodels;
		panim->headskins = ld->headskins;

		LoadAnim(panim);
	}
	else
	{
		MOHPC_LOG(Error, "TIKI_InitTiki: No valid animations found in %s.", ld->path);
		delete panim;
		return nullptr;
	}

	return panim;
}

const char* MOHPC::TIKIError::BadSetup::what() const
{
	return "Invalid setup section";
}