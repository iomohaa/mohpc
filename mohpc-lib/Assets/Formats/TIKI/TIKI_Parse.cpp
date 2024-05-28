#include <Shared.h>
#include <MOHPC/Common/Log.h>
#include <MOHPC/Assets/Formats/TIKI.h>
#include "TIKI_Private.h"
#include <string.h>
#include <algorithm>

using namespace MOHPC;

static constexpr char MOHPC_LOG_NAMESPACE[] = "tiki_parse";

bool TIKIReader::ParseSetup(dloaddef_t* ld)
{
	// Skip the setup token
	ld->tikiFile->GetToken(true);

	while (ld->tikiFile->TokenAvailable(true))
	{
		const char* token = ld->tikiFile->GetToken(true);

		if (!strHelpers::icmp(token, "scale"))
		{
			ld->loaddata.load_scale = ld->tikiFile->GetFloat(false);
		}
		else if (!strHelpers::icmp(token, "lod_scale"))
		{
			ld->loaddata.lod_scale = ld->tikiFile->GetFloat(false) / 5.0f;
		}
		else if (!strHelpers::icmp(token, "lod_bias"))
		{
			ld->loaddata.lod_bias = ld->tikiFile->GetFloat(false);
		}
		else if (!strHelpers::icmp(token, "skelmodel"))
		{
			token = ld->tikiFile->GetToken(false);
			ld->loaddata.skelmodel.push_back(ld->tikiFile->getCurrentScript()->getPath() + token);
		}
		else if (!strHelpers::icmp(token, "path"))
		{
			token = ld->tikiFile->GetToken(false);
			ld->tikiFile->getCurrentScript()->setPath(token);
		}
		else if (!strHelpers::icmp(token, "orgin"))
		{
			ld->loaddata.origin[0] = ld->tikiFile->GetFloat(false);
			ld->loaddata.origin[1] = ld->tikiFile->GetFloat(false);
			ld->loaddata.origin[2] = ld->tikiFile->GetFloat(false);
		}
		else if (!strHelpers::icmp(token, "lightoffset"))
		{
			ld->loaddata.origin[0] = ld->tikiFile->GetFloat(false);
			ld->loaddata.origin[1] = ld->tikiFile->GetFloat(false);
			ld->loaddata.origin[2] = ld->tikiFile->GetFloat(false);
		}
		else if (!strHelpers::icmp(token, "radius"))
		{
			ld->loaddata.radius = ld->tikiFile->GetFloat(false);
		}
		else if (!strHelpers::icmp(token, "surface"))
		{
			token = ld->tikiFile->GetToken(false);

			dloadsurface_t& surf = ld->loaddata.surfaces.emplace_back();
			surf.name = token;
			surf.flags = 0;
			surf.damage_multiplier = 0;

			while (ld->tikiFile->TokenAvailable(false))
			{
				token = ld->tikiFile->GetToken(false);
				if (!strHelpers::icmp(token, "flags"))
				{
					token = ld->tikiFile->GetToken(false);
					surf.flags = ParseSurfaceFlag(token);
				}
				else if (!strHelpers::icmp(token, "damage"))
				{
					surf.damage_multiplier = ld->tikiFile->GetFloat(false);
				}
				else if (!strHelpers::icmp(token, "shader"))
				{
					token = ld->tikiFile->GetToken(false);
					if (strHelpers::find(token, ".")) {
						surf.shader.push_back(ld->tikiFile->getCurrentScript()->getPath() + token);
					} else {
						surf.shader.push_back(token);
					}
				}
			}
		}
		else if (!strHelpers::icmp(token, "ischaracter"))
		{
			ld->bIsCharacter = true;
		}
		else if (!strHelpers::icmp(token, "case"))
		{
			if (!ParseCase(ld))
				return false;
		}
		else if (!strHelpers::icmp(token, "}"))
		{
			return true;
		}
		else
		{
			MOHPC_LOG(Error, "ParseSetup(): unknown setup command '%s' in '%s' on line %d, skipping line.", token, ld->tikiFile->getFilename().generic_string().c_str(), ld->tikiFile->GetLineNumber());
			
			while (ld->tikiFile->TokenAvailable(false)) {
				ld->tikiFile->GetToken(false);
			}
		}
	}

	return true;
}

void TIKIReader::ParseInitCommands(dloaddef_t* ld, tiki_init_cmd_list_t& cmdlist)
{
	// Skip current token
	ld->tikiFile->GetToken(true);

	while (ld->tikiFile->TokenAvailable(true))
	{
		const char* token = ld->tikiFile->GetToken(true);

		if (!strHelpers::icmp(token, "}"))
		{
			return;
		}

		dloadinitcmd_t& cmd = cmdlist.emplace_back();

		ld->tikiFile->UnGetToken();
		while (ld->tikiFile->TokenAvailable(false))
		{
			token = ld->tikiFile->GetToken(false);
			cmd.args.push_back(token);
		}
	}
}

void TIKIReader::ParseInit(dloaddef_t* ld)
{
	// Skip the init token
	ld->tikiFile->GetToken(true);

	while (ld->tikiFile->TokenAvailable(true))
	{
		const char* token = ld->tikiFile->GetToken(true);
		if (!strHelpers::icmp(token, "client"))
		{
			ParseInitCommands(ld, ld->loadclientinitcmds);
		}
		else if (!strHelpers::icmp(token, "server"))
		{
			ParseInitCommands(ld, ld->loadserverinitcmds);
		}
		else if (!strHelpers::icmp(token, "}"))
		{
			return;
		}
		else
		{
			MOHPC_LOG(Error, "ParseInit(): unknown init command '%s' in '%s' on line %d, skipping line.", token, ld->tikiFile->getFilename().generic_string().c_str(), ld->tikiFile->GetLineNumber());

			// Skip the current line
			while (ld->tikiFile->TokenAvailable(false)) {
				ld->tikiFile->GetToken(false);
			}
		}
	}
}

bool TIKIReader::ParseCase(dloaddef_t* ld)
{
	const char *token;
	bool isheadmodel;
	bool isheadskin;

__newcase:
	token = ld->tikiFile->GetToken(false);
	str key = token;
	isheadmodel = (!strHelpers::icmp(token, "headmodel")) ? true : false;
	isheadskin = (!strHelpers::icmp(token, "headskin")) ? true : false;

	while (1)
	{
		if (!ld->tikiFile->TokenAvailable(true))
		{
			MOHPC_LOG(Error, "ParseCase(): unexpected end of file while parsing 'case' switch in %s on line %d.", ld->tikiFile->getFilename().generic_string().c_str(), ld->tikiFile->GetLineNumber());
			return false;
		}

		token = ld->tikiFile->GetToken(true);
		if (!strHelpers::icmp(token, "case"))
		{
			goto __newcase;
		}
		else if (!strHelpers::icmp(token, "{"))
		{
			break;
		}

		if (isheadmodel && std::find(ld->headmodels.begin(), ld->headmodels.end(), token) == ld->headmodels.end()) {
			ld->headmodels.push_back(token);
		}

		if (isheadskin && std::find(ld->headskins.begin(), ld->headskins.end(), token) == ld->headskins.end()) {
			ld->headskins.push_back(token);
		}
	}

	ld->tikiFile->UnGetToken();
	if (!ParseSetup(ld))
	{
		return false;
	}

	return true;
}

void TIKIReader::ParseFrameCommands(dloaddef_t* ld, tiki_frame_cmd_list_t& cmdlist)
{
	bool usecurrentframe = false;
	const char *token;
	uint32_t framenum = 0;

	ld->tikiFile->GetToken(true);

	while (ld->tikiFile->TokenAvailable(true))
	{
		token = ld->tikiFile->GetToken(true);
		if (!strHelpers::icmp(token, "}"))
		{
			return;
		}

		dloadframecmd_t& cmd = cmdlist.emplace_back();

		if (!strHelpers::icmp(token, "start") || !strHelpers::icmp(token, "first"))
		{
			framenum = TF::FIRST;
		}
		else if (!strHelpers::icmp(token, "end"))
		{
			framenum = TF::END;
		}
		else if (!strHelpers::icmp(token, "last"))
		{
			framenum = TF::LAST;
		}
		else if (!strHelpers::icmp(token, "every"))
		{
			framenum = TF::EVERY;
		}
		else if (!strHelpers::icmp(token, "exit"))
		{
			framenum = TF::EXIT;
		}
		else if (!strHelpers::icmp(token, "entry") || !strHelpers::icmp(token, "enter"))
		{
			framenum = TF::ENTRY;
		}
		else if (!strHelpers::icmp(token, "("))
		{
			usecurrentframe = true;
			ld->tikiFile->UnGetToken();
		}
		else if (!strHelpers::icmp(token, ")"))
		{
			usecurrentframe = false;
			ld->tikiFile->UnGetToken();
		}
		else if (!usecurrentframe)
		{
			framenum = atoi(token);
		}
		else
		{
			ld->tikiFile->UnGetToken();
		}

		/*
		if (framenum < TF::LAST)
		{
			MOHPC_LOG(Error, "ParseFrameCommands(): illegal frame number %d on line %d in %s", framenum, ld->tikiFile->getFilename().generic_string().c_str(), ld->tikiFile->GetLineNumber());
			while (ld->tikiFile->TokenAvailable(false))
				ld->tikiFile->GetToken(false);
			continue;
		}
		*/

		cmd.frame_num = framenum;

		const TikiScriptPtr& tikiScript = ld->tikiFile->getCurrentScript();
		if (tikiScript)
		{
			cmd.location.reserve(tikiScript->getFilename().native().length() + 8 + 6);
			cmd.location = tikiScript->getFilename().generic_string();
			cmd.location += ", line: ";
			cmd.location += tikiScript->GetLineNumber();
		}

		while (ld->tikiFile->TokenAvailable(false))
		{
			token = ld->tikiFile->GetToken(false);
			cmd.args.push_back(token);
		}
	}
}

void TIKIReader::ParseAnimationCommands(dloaddef_t* ld, dloadanim_t* anim)
{
	while (ld->tikiFile->TokenAvailable(true))
	{
		const char* token = ld->tikiFile->GetToken(true);
		if (!strHelpers::icmp(token, "client"))
		{
			ParseFrameCommands(ld, anim->loadclientcmds);
		}
		else if (!strHelpers::icmp(token, "server"))
		{
			ParseFrameCommands(ld, anim->loadservercmds);
		}
		else if (!strHelpers::icmp(token, "}"))
		{
			return;
		}
		else
		{
			MOHPC_LOG(Error, "ParseAnimationCommands(): unknown anim command '%s' in '%s' on line %d, skipping line.", token, ld->tikiFile->getFilename().generic_string().c_str(), ld->tikiFile->GetLineNumber());
			
			while (ld->tikiFile->TokenAvailable(false)) {
				token = ld->tikiFile->GetToken(false);
			}
		}
	}
}

void TIKIReader::ParseAnimationFlags(dloaddef_t* ld, dloadanim_t* anim)
{
	while (ld->tikiFile->TokenAvailable(false))
	{
		const char* token = ld->tikiFile->GetToken(false);

		if (!strHelpers::icmp(token, "weight"))
		{
			anim->weight = ld->tikiFile->GetFloat(false);
			anim->flags |= TAF::RANDOM;
		}
		else if (!strHelpers::icmp(token, "deltadriven"))
		{
			anim->flags |= TAF::DELTADRIVEN;
		}
		else if (!strHelpers::icmp(token, "default_angles"))
		{
			anim->flags |= TAF::DEFAULT_ANGLES;
		}
		else if (!strHelpers::icmp(token, "notimecheck"))
		{
			anim->flags |= TAF::NOTIMECHECK;
		}
		else if (!strHelpers::icmp(token, "crossblend"))
		{
			anim->blendtime = ld->tikiFile->GetFloat(false);
		}
		else if (!strHelpers::icmp(token, "dontrepeate"))
		{
			anim->flags |= TAF::NOREPEAT;
		}
		else if (!strHelpers::icmp(token, "random"))
		{
			anim->flags |= TAF::RANDOM;
		}
		else if (!strHelpers::icmp(token, "autosteps_run"))
		{
			anim->flags |= TAF::AUTOSTEPS_RUN;
		}
		else if (!strHelpers::icmp(token, "autosteps_walk"))
		{
			anim->flags |= TAF::AUTOSTEPS_WALK;
		}
		else if (!strHelpers::icmp(token, "autosteps_dog"))
		{
			anim->flags |= TAF::AUTOSTEPS_DOG;
		}
		else
		{
			MOHPC_LOG(Error, "Unknown Animation flag %s for anim '%s' in %s", token, anim->alias.c_str(), ld->tikiFile->getFilename().generic_string().c_str());
		}
	}
}

void TIKIReader::ParseAnimationsFail(dloaddef_t* ld)
{
	int nestcount = 0;
	const char *token;

	while (ld->tikiFile->TokenAvailable(false))
	{
		ld->tikiFile->GetToken(false);
	}

	if (!ld->tikiFile->TokenAvailable(true))
	{
		return;
	}

	token = ld->tikiFile->GetToken(true);
	if (strHelpers::icmp(token, "{"))
	{
		ld->tikiFile->UnGetToken();
		return;
	}

	ld->tikiFile->UnGetToken();

	while (ld->tikiFile->TokenAvailable(true))
	{
		token = ld->tikiFile->GetToken(true);
		if (!strHelpers::icmp(token, "{"))
		{
			nestcount++;
		}
		else if (!strHelpers::icmp(token, "}"))
		{
			nestcount--;
			if (!nestcount) {
				return;
			}
		}
	}
}

bool TIKIReader::ParseIncludes(dloaddef_t* ld)
{
	bool b_incl = false;
	size_t depth = 0;

	while(ld->tikiFile->TokenAvailable(true))
	{
		const char* token = ld->tikiFile->GetToken(true);
		if ((!strHelpers::icmp(token, "{")))
		{
			break;
		}
		else if (isIncludeAllowed(token))
		{
			b_incl = true;
		}
	}

	if (b_incl)
	{
		// a condition successfully match so don't skip the include list
		return true;
	}

	// skip the entire section
	while (ld->tikiFile->TokenAvailable(true))
	{
		const char* token = ld->tikiFile->GetAndIgnoreLine(false);
		if (strHelpers::find(token, "{"))
		{
			depth++;
		}

		if (strHelpers::find(token, "}"))
		{
			if (!depth)
			{
				// section closed
				return false;
			}

			depth--;
		}
	}

	return false;
}

void TIKIReader::ParseAnimations(dloaddef_t* ld)
{
	bool b_mapspec = false;

	ld->tikiFile->GetToken(true);

	const TikiScriptPtr& tikiScript = ld->tikiFile->getCurrentScript();
	std::string fileName = tikiScript->getFilename().generic_string();

	while (ld->tikiFile->TokenAvailable(true))
	{
		const char* token = ld->tikiFile->GetToken(true);
		if (!strHelpers::icmp(token, "}"))
		{
			if (!b_mapspec) {
				return;
			}
			b_mapspec = false;
		}
		else if (!strHelpers::icmp(token, "$mapspec"))
		{
			while (ld->tikiFile->TokenAvailable(true))
			{
				const char* token = ld->tikiFile->GetToken(true);
				if ((!strHelpers::icmp(token, "{")))
				{
					break;
				}
				else if (isIncludeAllowed(token))
				{
					b_mapspec = true;
				}
			}

			if (!b_mapspec)
			{
				size_t depth = 0;
				while (ld->tikiFile->TokenAvailable(true))
				{
					token = ld->tikiFile->GetToken(true);
					if (strHelpers::find(token, "{"))
					{
						depth++;
					}

					if (strHelpers::find(token, "}"))
					{
						if (!depth) {
							continue;
						}

						depth--;
					}
				}
			}
		}
		else
		{
			auto pair = ld->loadanims.emplace(token);

			dloadanim_t& anim = const_cast<dloadanim_t&>(*pair.first);
			if (!pair.second)
			{
				// already exist
				// reset command list
				anim.init();
				anim.loadclientcmds.clear();
				anim.loadservercmds.clear();
			}

			token = ld->tikiFile->GetToken(false);

			anim.name.reserve(tikiScript->getPath().length() + strHelpers::len(token) + 1);
			anim.name = tikiScript->getPath();
			anim.name += token;

			anim.location.reserve(fileName.length() + 8 + 6);
			anim.location = fileName;
			anim.location += ", line: ";
			anim.location += std::to_string(tikiScript->GetLineNumber());

			ParseAnimationFlags(ld, &anim);

			if (ld->tikiFile->TokenAvailable(true))
			{
				token = ld->tikiFile->GetToken(true);
				if (!strHelpers::icmp(token, "{"))
				{
					ParseAnimationCommands(ld, &anim);
				}
				else
				{
					ld->tikiFile->UnGetToken();
				}
			}
		}
	}

	MOHPC_LOG(Error, "no closing braces");
}

void TIKIReader::ParseQuaked(dloaddef_t* ld)
{
	ld->loaddata.quaked.name = ld->tikiFile->GetToken(true);
	ld->tikiFile->GetVector(true, (float*)&ld->loaddata.quaked.color);

	const char* undefinedBounds = ld->tikiFile->GetToken(true);
	if (*undefinedBounds != '?')
	{
		ld->tikiFile->UnGetToken();
		ld->tikiFile->GetVector(true, (float*)&ld->loaddata.quaked.mins);
		ld->tikiFile->GetVector(true, (float*)&ld->loaddata.quaked.maxs);
	}

	while (ld->tikiFile->TokenAvailable(true))
	{
		const char* token = ld->tikiFile->GetToken(true);
		if (!strHelpers::icmp(token, "*/")) {
			return;
		}

		ld->loaddata.quaked.spawnFlags.push_back(token);
	}
}

int32_t TIKIReader::ParseSurfaceFlag(const char* token)
{
	int flags = 0;

	if (!strHelpers::icmp(token, "skin1"))
	{
		flags = TS::SKIN1;
	}
	else if (!strHelpers::icmp(token, "skin2"))
	{
		flags = TS::SKIN2;
	}
	else if (!strHelpers::icmp(token, "skin3"))
	{
		flags = TS::SKIN3;
	}
	else if (!strHelpers::icmp(token, "nodraw"))
	{
		flags = TS::NODRAW;
	}
	else if (!strHelpers::icmp(token, "nodamage"))
	{
		flags = TS::NODAMAGE;
	}
	else if (!strHelpers::icmp(token, "crossfade"))
	{
		flags = TS::CROSSFADE;
	}
	else if (!strHelpers::icmp(token, "nomipmaps"))
	{
		flags = TS::NOMIPMAPS;
	}
	else if (!strHelpers::icmp(token, "nopicmip"))
	{
		flags = TS::NOPICMIP;
	}
	else
	{
		//MOHPC_LOG(Error, "Unknown surface flag '%s'", token);
	}

	return flags;
}

void TIKIReader::InitSetup(dloaddef_t* ld)
{
	ld->tikiFile = nullptr;
	ld->bInIncludesSection = false;
	ld->bIsCharacter = false;

	ld->loaddata.load_scale = 0.f;
	ld->loaddata.lod_scale = 0.f;
	ld->loaddata.lod_bias = 0.f;
	VectorClear(ld->loaddata.origin);
	VectorClear(ld->loaddata.lightoffset);
	ld->loaddata.radius = 0.f;
}

bool TIKIReader::LoadSetupCase(TIKI& tiki, const dloaddef_t* ld, tiki_surface_list_t& loadsurfaces)
{
	tiki.setLoadScale(ld->loaddata.load_scale);
	tiki.setLodScale(ld->loaddata.lod_scale);
	tiki.setLodBias(ld->loaddata.lod_bias);

	vec3_t loadOrigin;
	VectorCopy(ld->loaddata.origin, loadOrigin);
	tiki.setLoadOrigin(loadOrigin);

	vec3_t lightOffset;
	VectorCopy(ld->loaddata.lightoffset, lightOffset);
	tiki.setLightOffset(lightOffset);
	tiki.setRadius(ld->loaddata.radius);

	loadsurfaces = ld->loaddata.surfaces;

	size_t numSurfaces = 0;

	std::vector<SkeletonPtr>& meshes = tiki.getMeshes();
	for (auto& skelmodel : ld->loaddata.skelmodel)
	{
		const SkeletonPtr Mesh = GetAssetManager()->readAsset<SkeletonReader>(skelmodel.c_str());
		if (Mesh)
		{
			meshes.push_back(Mesh);
			numSurfaces += Mesh->GetNumSurfaces();
		}
	}

	std::vector<TIKISurface>& surfaces = tiki.getSurfaces();
	surfaces.resize(numSurfaces);

	return true;
}

bool TIKIReader::LoadSetup(TIKI& tiki, const dloaddef_t* ld, tiki_surface_list_t& loadsurfaces)
{
	return LoadSetupCase(tiki, ld, loadsurfaces);
}

bool TIKIReader::isIncludeAllowed(const char* includeName)
{
	if (!allowedIncludes || !numAllowedIncludes)
	{
		// allow everything by default
		return true;
	}

	for (size_t i = 0; i < numAllowedIncludes; ++i)
	{
		if (!strHelpers::icmp(includeName, allowedIncludes[i]))
		{
			// the include name was found in allowed includes
			return true;
		}
	}

	// the include is not in the allowed list
	return false;
}
