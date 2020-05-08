#include <Shared.h>
#include <MOHPC/Formats/TIKI.h>
#include "TIKI_Private.h"
#include "../Skel/SkelPrivate.h"
#include <string.h>

using namespace MOHPC;

bool TIKI::Load()
{
	if (tikianim)
	{
		delete tikianim;
		tikianim = NULL;
	}

	dloaddef_t loaddef;
	bool bResult;

	const char *Fname = GetFilename().c_str();

	loaddef.tikiFile = nullptr;

	if (bResult = LoadTIKIAnim(Fname, &loaddef))
	{
		bResult = LoadTIKIModel(Fname, &loaddef);
	}

	if (loaddef.tikiFile)
	{
		HashCopy(loaddef.tikiFile.get());
		loaddef.tikiFile->SetCurrentScript(nullptr);
	}

	FreeStorage(&loaddef);
	return bResult;
}

void TIKI::FixFrameNum(TIKIAnim *ptiki, SkeletonAnimation *animData, TIKIAnim::Command *cmd, const char *alias)
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
		//TIKI_Error("TIKI_FixFrameNum: illegal frame number %d (total: %d) in anim '%s' in '%s'\n", cmd->frame_num, animData->numFrames, alias, ptiki->name);
		cmd->frame_num = 0;
	}
}

void TIKI::LoadAnim(TIKIAnim* ptikianim)
{
	size_t i, j;
	TIKIAnim::AnimDef *panimdef;
	SkeletonAnimation* animData;

	for (i = 0; i < ptikianim->animdefs.size(); i++)
	{
		panimdef = &ptikianim->animdefs[i];
		animData = panimdef->animData.get();
		if (animData)
		{
			for (j = 0; j < panimdef->server_cmds.size(); j++)
			{
				FixFrameNum(ptikianim, animData, &panimdef->server_cmds[j], panimdef->alias.c_str());
			}

			for (j = 0; j < panimdef->client_cmds.size(); j++)
			{
				FixFrameNum(ptikianim, animData, &panimdef->client_cmds[j], panimdef->alias.c_str());
			}
		}
	}
}

bool MOHPC::TIKI::LoadTIKIAnim(const char* Filename, dloaddef_t* ld)
{
	TIKIAnim* TikiAnim = NULL;
	const char *token;
	Vector tempVec;
	str s;

	InitSetup(ld);

	ld->tikiFile = GetAssetManager()->LoadAsset<TikiScript>(Filename);
	if (!ld->tikiFile)
	{
		return false;
	}

	ld->path = Filename;

	token = ld->tikiFile->GetToken(true);
	if (strcmp(token, "TIKI"))
	{
		//TIKI_Error("TIKI_LoadTIKIfile: def file %s has wrong header (%s should be TIKI)\n", ld->tikiFile->Filename(), token);
		return false;
	}

	ld->bInIncludesSection = false;
	ld->tikiFile->SetAllowExtendedComment(true);

	while (ld->tikiFile->TokenAvailable(true))
	{
		token = ld->tikiFile->GetToken(true);

		if (!stricmp(token, "setup"))
		{
			if (!ParseSetup(ld))
			{
				return false;
			}
		}
		else if (!stricmp(token, "init"))
		{
			ParseInit(ld);
		}
		else if (!stricmp(token, "animations"))
		{
			ParseAnimations(ld);
		}
		else if (!stricmp(token, "includes"))
		{
			if (!ld->bInIncludesSection)
			{
				ld->bInIncludesSection = ParseIncludes(ld);
			}
			else
			{
				/*TIKI_Error("TIKI_LoadTIKIfile: Nested Includes section in %s on line %d, the animations will be fubar\n",
					token,
					ld->tikiFile->GetLineNumber(),
					ld->tikiFile->Filename());*/
			}
		}
		else if (!stricmp(token, "}") && ld->bInIncludesSection)
		{
			ld->bInIncludesSection = false;
		}
		else if (!stricmp(token, "/*QUAKED"))
		{
			ParseQuaked(ld);
		}
		else
		{
			//TIKI_Error("TIKI_LoadTIKIfile: unknown section %s in %s online %d, skipping line.\n", token, ld->tikiFile->Filename(), ld->tikiFile->GetLineNumber());

			// skip the current line
			while (ld->tikiFile->TokenAvailable(false)) {
				ld->tikiFile->GetToken(false);
			}
		}
	}

	if (ld->bInIncludesSection)
	{
		//TIKI_Error("TIKI_LoadTIKIfile: Include section in %s did not terminate\n", ld->tikiFile->Filename());
	}

	if (ld->loadanims.size())
	{
		TikiAnim = InitTiki(ld);
		if (TikiAnim)
		{
			tempVec = TikiAnim->maxs - TikiAnim->mins;
			if (tempVec.length() > 100000.0f)
			{
				TikiAnim->mins = Vector(-4.f, -4.f, -4.f);
				TikiAnim->maxs = Vector(4.f, 4.f, 4.f);
			}

			tikianim = TikiAnim;
		}
		else
		{
			return false;
		}
	}
	else
	{
		//TIKI_Error("TIKI_LoadTIKIfile: No valid animations found in %s.\n", ld->tikiFile->Filename());
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

		const char* strptr = strchr(loadsurf->name.c_str(), '*');

		size_t surfOffset = 0;

		if (strptr || !stricmp(loadsurf->name.c_str(), "all"))
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
						&& !strnicmp(loadsurf->name.c_str(), surf->name.c_str(), strptr - loadsurf->name.c_str()))
						|| !stricmp(loadsurf->name.c_str(), "all"))
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

					if (!stricmp(loadsurf->name.c_str(), surf->name.c_str()))
					{
						SetupIndividualSurface(tikianim->name.c_str(), tikiSurf, surf->name.c_str(), loadsurf);
						if (!tikiSurf->name[0])
						{
							//TIKI_Warning("TIKI_InitTiki: Surface %i in %s(referenced in %s) has no name!  Please investigate and fix\n", k, skelmodel->name, name);
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
			//TIKI_Warning("TIKI_InitTiki: could not find surface '%s' in '%s' (check referenced skb/skd files).\n", loadsurf, tikianim->name);
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
	TIKIAnim::Command *pcmds;
	size_t i, k;
	size_t j;
	TIKIAnim *panim;
	dloadanim_t *anim;
	bool bModelBoundsSet = false;
	Container<size_t> order;

	panim = new TIKIAnim;
	ClearBounds(panim->mins, panim->maxs);
	panim->bIsCharacter = ld->bIsCharacter;
	panim->name = ld->path;

	panim->server_initcmds.resize(ld->loadserverinitcmds.size());

	// Process server init commands
	for (i = 0; i < ld->loadserverinitcmds.size(); i++)
	{
		pcmds = &panim->server_initcmds[i];

		for (j = 0; j < ld->loadserverinitcmds[i].args.size(); j++)
		{
			pcmds->args.push_back(ld->loadserverinitcmds[i].args[j]);
		}
	}

	panim->client_initcmds.resize(ld->loadclientinitcmds.size());

	// Process client init commands
	for (i = 0; i < ld->loadclientinitcmds.size(); i++)
	{
		pcmds = &panim->client_initcmds[i];

		for (j = 0; j < ld->loadclientinitcmds[i].args.size(); j++)
		{
			pcmds->args.push_back(ld->loadclientinitcmds[i].args[j]);
		}
	}

	GetAnimOrder(ld, order);

	size_t numAnims = ld->loadanims.size();

	// Process anim commands
	for (i = 0; i < numAnims; i++)
	{
		anim = &ld->loadanims[order[i]];
		SkeletonAnimationPtr data = GetAssetManager()->LoadAsset<SkeletonAnimation>(anim->name.c_str()); //SkeletonAnimation::RegisterAnim(anim->name.c_str());

		if (!data)
		{
			//TIKI_Error("TIKI_InitTiki: Failed to load animation '%s' at %s\n", anim->name, anim->location);
			continue;
		}

		TIKIAnim::AnimDef animdef;
		animdef.alias = anim->alias;
		animdef.weight = anim->weight;
		animdef.flags = anim->flags;
		animdef.animData = data;

		if (!stricmp(animdef.alias.c_str(), "idle"))
		{
			data->GetBounds(panim->mins, panim->maxs);
			bModelBoundsSet = true;
		}


		if (anim->flags & TAF_RANDOM)
		{
			j = animdef.alias.length();
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
				//TIKI_DPrintf("TIKI_InitTiki: Random animation name '%s' should end with a number\n", animdef.alias);
			}
		}

		animdef.blendtime = anim->blendtime;

		animdef.server_cmds.resize(anim->loadservercmds.size());

		// Process server anim commands
		for (j = 0; j < anim->loadservercmds.size(); j++)
		{
			pcmds = &animdef.server_cmds[j];
			pcmds->frame_num = anim->loadservercmds[j].frame_num;

			for (k = 0; k < anim->loadservercmds[j].args.size(); k++)
			{
				pcmds->args.push_back(anim->loadservercmds[j].args[k]);
			}
		}

		animdef.client_cmds.resize(anim->loadclientcmds.size());

		// Process server anim commands
		for (j = 0; j < anim->loadclientcmds.size(); j++)
		{
			pcmds = &animdef.client_cmds[j];
			pcmds->frame_num = anim->loadclientcmds[j].frame_num;

			for (k = 0; k < anim->loadclientcmds[j].args.size(); k++)
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
			//TIKI_DPrintf("TIKI_InitTiki: no 'idle' animation found, model bounds not set for %s\n", ld->path);
		}

		panim->headmodels = ld->headmodels;
		panim->headskins = ld->headskins;

		LoadAnim(panim);
	}
	else
	{
		//TIKI_Error("TIKI_InitTiki: No valid animations found in %s.\n", ld->path);
		panim = NULL;
	}

	return panim;
}
