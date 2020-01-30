#include <Shared.h>
#include <MOHPC/Formats/TIKI.h>
#include "TIKI_Private.h"
#include <string.h>
#include <algorithm>

using namespace MOHPC;
using namespace std;

bool TIKI::ParseSetup(dloaddef_t* ld)
{
	const char *token;
	float load_scale;
	float lod_scale;
	float lod_bias;
	float tmpFloat;
	Vector tmpVec;
	int tmpInt;
	size_t length;

	// Skip the setup token
	ld->tikiFile->GetToken(true);

	while (ld->tikiFile->TokenAvailable(true))
	{
		token = ld->tikiFile->GetToken(true);

		if (!stricmp(token, "scale"))
		{
			load_scale = ld->tikiFile->GetFloat(false);
			ld->loaddata.load_scale = load_scale;
		}
		else if (!stricmp(token, "lod_scale"))
		{
			lod_scale = ld->tikiFile->GetFloat(false) / 5.0f;
			ld->loaddata.lod_scale = lod_scale;
		}
		else if (!stricmp(token, "lod_bias"))
		{
			lod_bias = ld->tikiFile->GetFloat(false);
			ld->loaddata.lod_bias = lod_bias;
		}
		else if (!stricmp(token, "skelmodel"))
		{
			token = ld->tikiFile->GetToken(false);
			ld->loaddata.skelmodel.push_back(string(ld->tikiFile->currentScript->path) + token);
		}
		else if (!stricmp(token, "path"))
		{
			token = ld->tikiFile->GetToken(false);
			strcpy(ld->tikiFile->currentScript->path, token);
			length = strlen(ld->tikiFile->currentScript->path);
			token = ld->tikiFile->currentScript->path + length - 1;

			if (*token != '/' && *token != '\\')
			{
				strcat(ld->tikiFile->currentScript->path, "/");
			}
		}
		else if (!stricmp(token, "orgin"))
		{
			tmpVec[0] = ld->tikiFile->GetFloat(false);
			tmpVec[1] = ld->tikiFile->GetFloat(false);
			tmpVec[2] = ld->tikiFile->GetFloat(false);
			ld->loaddata.origin = tmpVec;
		}
		else if (!stricmp(token, "lightoffset"))
		{
			tmpVec[0] = ld->tikiFile->GetFloat(false);
			tmpVec[1] = ld->tikiFile->GetFloat(false);
			tmpVec[2] = ld->tikiFile->GetFloat(false);
			ld->loaddata.origin = tmpVec;
		}
		else if (!stricmp(token, "radius"))
		{
			tmpFloat = ld->tikiFile->GetFloat(false);
			ld->loaddata.radius = tmpFloat;
		}
		else if (!stricmp(token, "surface"))
		{
			token = ld->tikiFile->GetToken(false);
			dloadsurface_t surf;
			surf.name = token;
			surf.flags = 0;
			surf.damage_multiplier = 0;
			while (ld->tikiFile->TokenAvailable(false))
			{
				token = ld->tikiFile->GetToken(false);
				if (!stricmp(token, "flags"))
				{
					token = ld->tikiFile->GetToken(false);
					tmpInt = ParseSurfaceFlag(token);
					surf.flags = tmpInt;
				}
				else if (!stricmp(token, "damage"))
				{
					tmpFloat = ld->tikiFile->GetFloat(false);
					surf.damage_multiplier = tmpFloat;
				}
				else if (!stricmp(token, "shader"))
				{
					token = ld->tikiFile->GetToken(false);
					if (strstr(token, "."))
					{
						surf.shader.push_back(string(ld->tikiFile->currentScript->path) + token);
					}
					else
					{
						surf.shader.push_back(token);
					}
				}
			}
			ld->loaddata.surfaces.push_back(surf);
		}
		else if (!stricmp(token, "ischaracter"))
		{
			ld->bIsCharacter = true;
		}
		else if (!stricmp(token, "case"))
		{
			if (!ParseCase(ld))
				return false;
		}
		else if (!stricmp(token, "}"))
		{
			return true;
		}
		else
		{
			//TIKI_Error("TIKI_ParseSetup: unknown setup command '%s' in '%s' on line %d, skipping line.\n", token, ld->tikiFile->Filename(), ld->tikiFile->GetLineNumber());
			while (ld->tikiFile->TokenAvailable(false))
				ld->tikiFile->GetToken(false);
		}
	}

	return true;
}

void TIKI::ParseInitCommands(dloaddef_t* ld, vector<dloadinitcmd_t>& cmdlist)
{
	const char *token;
	vector<string> szArgs;

	// Skip current token
	ld->tikiFile->GetToken(true);

	while (ld->tikiFile->TokenAvailable(true))
	{
		token = ld->tikiFile->GetToken(true);

		if (!stricmp(token, "}"))
		{
			break;
		}

		dloadinitcmd_t cmd;

		ld->tikiFile->UnGetToken();
		while (ld->tikiFile->TokenAvailable(false))
		{
			token = ld->tikiFile->GetToken(false);
			cmd.args.push_back(token);
		}

		cmdlist.push_back(cmd);
	}
}

void TIKI::ParseInit(dloaddef_t* ld)
{
	const char *token;

	// Skip the init token
	ld->tikiFile->GetToken(true);

	while (ld->tikiFile->TokenAvailable(true))
	{
		token = ld->tikiFile->GetToken(true);
		if (!stricmp(token, "client"))
		{
			ParseInitCommands(ld, ld->loadclientinitcmds);
		}
		else if (!stricmp(token, "server"))
		{
			ParseInitCommands(ld, ld->loadserverinitcmds);
		}
		else if (!stricmp(token, "}"))
		{
			break;
		}
		else
		{
			//TIKI_Error("TIKI_ParseInit: unknown init command %s in %s on line %d, skipping line.\n", token, ld->tikiFile->Filename(), ld->tikiFile->GetLineNumber());

			// Skip the current line
			while (ld->tikiFile->TokenAvailable(false))
				ld->tikiFile->GetToken(false);
		}
	}
}

bool TIKI::ParseCase(dloaddef_t* ld)
{
	const char *token;
	bool isheadmodel;
	bool isheadskin;

__newcase:
	token = ld->tikiFile->GetToken(false);
	string key = token;
	isheadmodel = (!stricmp(token, "headmodel")) ? true : false;
	isheadskin = (!stricmp(token, "headskin")) ? true : false;

	while (1)
	{
		if (!ld->tikiFile->TokenAvailable(true))
		{
			//TIKI_Error("TIKI_ParseSetup: unexpected end of file while parsing 'case' switch in %s on line %d.\n", ld->tikiFile->Filename(), ld->tikiFile->GetLineNumber());
			return false;
		}

		token = ld->tikiFile->GetToken(true);
		if (!stricmp(token, "case"))
		{
			goto __newcase;
		}
		else if (!stricmp(token, "{"))
		{
			break;
		}

		ld->keyvalues[key] = token;

		if (isheadmodel && std::find(ld->headmodels.begin(), ld->headmodels.end(), token) == ld->headmodels.end())
		{
			ld->headmodels.push_back(token);
		}

		if (isheadskin && std::find(ld->headskins.begin(), ld->headskins.end(), token) == ld->headskins.end())
		{
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

void TIKI::ParseFrameCommands(dloaddef_t* ld, vector<dloadframecmd_t>& cmdlist)
{
	bool usecurrentframe = false;
	const char *token;
	int framenum = 0;

	ld->tikiFile->GetToken(true);

	while (ld->tikiFile->TokenAvailable(true))
	{
		token = ld->tikiFile->GetToken(true);
		if (!stricmp(token, "}"))
		{
			break;
		}

		dloadframecmd_t cmd;

		if (!stricmp(token, "start") || !stricmp(token, "first"))
		{
			framenum = TIKI_FRAME_FIRST;
		}
		else if (!stricmp(token, "end"))
		{
			framenum = TIKI_FRAME_END;
		}
		else if (!stricmp(token, "last"))
		{
			framenum = TIKI_FRAME_LAST;
		}
		else if (!stricmp(token, "every"))
		{
			framenum = TIKI_FRAME_EVERY;
		}
		else if (!stricmp(token, "exit"))
		{
			framenum = TIKI_FRAME_EXIT;
		}
		else if (!stricmp(token, "entry") || !stricmp(token, "enter"))
		{
			framenum = TIKI_FRAME_ENTRY;
		}
		else if (!stricmp(token, "("))
		{
			usecurrentframe = true;
			ld->tikiFile->UnGetToken();
		}
		else if (!stricmp(token, ")"))
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

		if (framenum < TIKI_FRAME_LAST)
		{
			//TIKI_Error("TIKI_ParseFrameCommands: illegal frame number %d on line %d in %s\n", framenum, ld->tikiFile->Filename(), ld->tikiFile->GetLineNumber());
			while (ld->tikiFile->TokenAvailable(false))
				ld->tikiFile->GetToken(false);
			continue;
		}

		cmd.frame_num = framenum;
		if (ld->tikiFile->currentScript)
		{
			cmd.location = ld->tikiFile->Filename();
			cmd.location += string(", line: ");
			cmd.location += ld->tikiFile->GetLineNumber();
		}

		while (ld->tikiFile->TokenAvailable(false))
		{
			token = ld->tikiFile->GetToken(false);
			cmd.args.push_back(token);
		}

		cmdlist.push_back(cmd);
	}
}

void TIKI::ParseAnimationCommands(dloaddef_t* ld, dloadanim_t* anim)
{
	const char *token;

	while (ld->tikiFile->TokenAvailable(true))
	{
		token = ld->tikiFile->GetToken(true);
		if (!stricmp(token, "client"))
		{
			ParseFrameCommands(ld, anim->loadclientcmds);
		}
		else if (!stricmp(token, "server"))
		{
			ParseFrameCommands(ld, anim->loadservercmds);
		}
		else if (!stricmp(token, "}"))
		{
			break;
		}
		else
		{
			//TIKI_Error("TIKI_ParseAnimationCommands: unknown anim command '%s' in '%s' on line %d, skipping line.\n", token, ld->tikiFile->Filename(), ld->tikiFile->GetLineNumber());
			while (ld->tikiFile->TokenAvailable(false))
				token = ld->tikiFile->GetToken(false);
		}
	}
}

void TIKI::ParseAnimationFlags(dloaddef_t* ld, dloadanim_t* anim)
{
	const char *token;

	anim->weight = 1.0f;
	anim->blendtime = 0.2f;
	anim->flags = 0;

	while (ld->tikiFile->TokenAvailable(false))
	{
		token = ld->tikiFile->GetToken(false);

		if (!stricmp(token, "weight"))
		{
			anim->weight = ld->tikiFile->GetFloat(false);
			anim->flags |= TAF_RANDOM;
		}
		else if (!stricmp(token, "deltadriven"))
		{
			anim->flags |= TAF_DELTADRIVEN;
		}
		else if (!stricmp(token, "default_angles"))
		{
			anim->flags |= TAF_DEFAULT_ANGLES;
		}
		else if (!stricmp(token, "notimecheck"))
		{
			anim->flags |= TAF_NOTIMECHECK;
		}
		else if (!stricmp(token, "crossblend"))
		{
			anim->blendtime = ld->tikiFile->GetFloat(false);
		}
		else if (!stricmp(token, "dontrepeate"))
		{
			anim->flags |= TAF_NOREPEAT;
		}
		else if (!stricmp(token, "random"))
		{
			anim->flags |= TAF_RANDOM;
		}
		else if (!stricmp(token, "autosteps_run"))
		{
			anim->flags |= TAF_AUTOSTEPS_RUN;
		}
		else if (!stricmp(token, "autosteps_walk"))
		{
			anim->flags |= TAF_AUTOSTEPS_WALK;
		}
		else if (!stricmp(token, "autosteps_dog"))
		{
			anim->flags |= TAF_AUTOSTEPS_DOG;
		}
		else
		{
			//TIKI_Error("Unknown Animation flag %s for anim '%s' in %s\n", token, anim->alias, TikiScript::currentScript->Filename());
		}
	}
}

void TIKI::ParseAnimationsFail(dloaddef_t* ld)
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
	if (stricmp(token, "{"))
	{
		ld->tikiFile->UnGetToken();
		return;
	}

	ld->tikiFile->UnGetToken();

	while (ld->tikiFile->TokenAvailable(true))
	{
		token = ld->tikiFile->GetToken(true);
		if (!stricmp(token, "{"))
		{
			nestcount++;
		}
		else if (!stricmp(token, "}"))
		{
			nestcount--;
			if (!nestcount)
				break;
		}
	}
}

bool TIKI::ParseIncludes(dloaddef_t* ld)
{
	const char *token;
	bool b_incl = false;
	const char *mapname;
	int depth = 0;

	token = ld->tikiFile->GetToken(true);
	mapname = "utils"; // FIXME: get current map name

	while (1)
	{
		if (!strncmp(token, mapname, strlen(token))
			|| !strncmp(token, "spearheadserver", strlen(token))
			|| !strncmp(token, "breakthroughserver", strlen(token)))
		{
			b_incl = true;
		}
		else if ((!stricmp(token, "{") || !ld->tikiFile->TokenAvailable(true)))
		{
			break;
		}

		token = ld->tikiFile->GetToken(true);
	}

	if (b_incl)
	{
		return true;
	}

	while (ld->tikiFile->TokenAvailable(true))
	{
		token = ld->tikiFile->GetAndIgnoreLine(false);
		if (strstr(token, "{"))
		{
			depth++;
		}

		if (strstr(token, "}"))
		{
			if (!depth)
			{
				break;
			}

			depth--;
		}
	}

	return false;
}

void TIKI::ParseAnimations(dloaddef_t* ld)
{
	const char *token;
	bool b_mapspec = false;
	const char *mapname;
	size_t depth = 0;

	ld->tikiFile->GetToken(true);

	while (ld->tikiFile->TokenAvailable(true))
	{
		token = ld->tikiFile->GetToken(true);
		if (!stricmp(token, "}"))
		{
			return;
		}
		else if (!stricmp(token, "$mapspec"))
		{
			token = ld->tikiFile->GetToken(true);
			mapname = "utils"; // FIXME

			while (ld->tikiFile->TokenAvailable(true))
			{
				if (!strncmp(token, mapname, strlen(token)))
				{
					b_mapspec = true;
				}
				else if (!stricmp(token, "{"))
				{
					break;
				}

				token = ld->tikiFile->GetToken(true);
			}

			if (!b_mapspec)
			{
				while (ld->tikiFile->TokenAvailable(true))
				{
					token = ld->tikiFile->GetToken(true);
					if (strstr(token, "{"))
					{
						depth++;
					}

					if (strstr(token, "}"))
					{
						if (!depth)
						{
							continue;
						}

						depth--;
					}
				}

				return;
			}
		}
		else
		{
			depth = strlen(token);
			if (depth < 48)
			{
				dloadanim_t anim;
				anim.alias = token;

				token = ld->tikiFile->GetToken(false);
				anim.name = string(ld->tikiFile->currentScript->path) + token;

				if (ld->tikiFile->currentScript)
				{
					anim.location += ld->tikiFile->currentScript->Filename();
					anim.location += ", line: ";
					anim.location += std::to_string(ld->tikiFile->currentScript->GetLineNumber());
				}

				ParseAnimationFlags(ld, &anim);

				if (ld->tikiFile->TokenAvailable(true))
				{
					token = ld->tikiFile->GetToken(true);
					if (!stricmp(token, "{"))
					{
						ParseAnimationCommands(ld, &anim);
					}
					else
					{
						ld->tikiFile->UnGetToken();
					}
				}

				ld->loadanims.push_back(anim);
			}
			else
			{
				//TIKI_Error("TIKI_ParseAnimations: Anim alias name %s is too long in %s.\n", token, ld->tikiFile->Filename());
				ParseAnimationsFail(ld);
			}
		}
	}
}

void TIKI::ParseQuaked(dloaddef_t* ld)
{
	quakedSection.name = ld->tikiFile->GetToken(true);
	ld->tikiFile->GetVector(true, (float*)&quakedSection.color);

	const char* undefinedBounds = ld->tikiFile->GetToken(true);
	if (*undefinedBounds != '?')
	{
		ld->tikiFile->UnGetToken();
		ld->tikiFile->GetVector(true, (float*)&quakedSection.mins);
		ld->tikiFile->GetVector(true, (float*)&quakedSection.maxs);
	}

	while (ld->tikiFile->TokenAvailable(true))
	{
		const char* token = ld->tikiFile->GetToken(true);
		if (!stricmp(token, "*/"))
		{
			break;
		}

		quakedSection.spawnFlags.push_back(token);
	}
}

int32_t TIKI::ParseSurfaceFlag(const char* token)
{
	int flags = 0;

	if (!stricmp(token, "skin1"))
	{
		flags = TIKI_SURF_SKIN1;
	}
	else if (!stricmp(token, "skin2"))
	{
		flags = TIKI_SURF_SKIN2;
	}
	else if (!stricmp(token, "skin3"))
	{
		flags = TIKI_SURF_SKIN3;
	}
	else if (!stricmp(token, "nodraw"))
	{
		flags = TIKI_SURF_NODRAW;
	}
	else if (!stricmp(token, "nodamage"))
	{
		flags = TIKI_SURF_NODAMAGE;
	}
	else if (!stricmp(token, "crossfade"))
	{
		flags = TIKI_SURF_CROSSFADE;
	}
	else if (!stricmp(token, "nomipmaps"))
	{
		flags = TIKI_SURF_NOMIPMAPS;
	}
	else if (!stricmp(token, "nopicmip"))
	{
		flags = TIKI_SURF_NOPICMIP;
	}
	else
	{
		//TIKI_Error("Unknown surface flag %s\n", token);
	}

	return flags;
}

void TIKI::InitSetup(dloaddef_t* ld)
{
	ld->tikiFile = NULL;
	ld->bInIncludesSection = false;
	ld->bIsCharacter = false;

	ld->loaddata.load_scale = 0.f;
	ld->loaddata.lod_scale = 0.f;
	ld->loaddata.lod_bias = 0.f;
	ld->loaddata.origin = Vector(0.f, 0.f, 0.f);
	ld->loaddata.lightoffset = Vector(0.f, 0.f, 0.f);
	ld->loaddata.radius = 0.f;
}

bool TIKI::LoadSetupCase(const char *filename, const dloaddef_t* ld, vector<dloadsurface_t>& loadsurfaces)
{
	this->loadScale = ld->loaddata.load_scale;
	this->lodScale = ld->loaddata.lod_scale;
	this->lodBias = ld->loaddata.lod_bias;
	this->loadOrigin = ld->loaddata.origin;
	this->lightOffset = ld->loaddata.lightoffset;
	this->radius = ld->loaddata.radius;

	loadsurfaces = ld->loaddata.surfaces;

	size_t numSurfaces = 0;

	for (auto& skelmodel : ld->loaddata.skelmodel)
	{
		std::shared_ptr<Skeleton> Mesh = GetAssetManager()->LoadAsset<Skeleton>(skelmodel.c_str()); //Skeleton::RegisterSkel(skelmodel.c_str());
		if (Mesh)
		{
			meshes.push_back(Mesh);
			numSurfaces += Mesh->GetNumSurfaces();
		}
	}

	surfaces.resize(numSurfaces);

	return true;
}

bool TIKI::LoadSetup(const char *filename, const dloaddef_t* ld, vector<dloadsurface_t>& loadsurfaces)
{
	return LoadSetupCase(filename, ld, loadsurfaces);
}
