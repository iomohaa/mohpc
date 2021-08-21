#include "TIKI_Private.h"

#include <cassert>

using namespace MOHPC;

MOHPC_OBJECT_DEFINITION(TIKI);
TIKI::TIKI(const fs::path& path, const SharedPtr<SkeletorManager>& skeletorManPtr, TIKIAnim* panim)
	: Asset(path)
	, skeletorManager(skeletorManPtr)
	, tikianim(panim)
{
	loadScale = 0.f;
	lodScale = 0.f;
	lodBias = 0.f;
	VectorClear(lightOffset);
	VectorClear(loadOrigin);
	radius = 0.f;
}

TIKI::~TIKI()
{
	if (tikianim)
	{
		// this tiki anim was allocated using a single buffer
		tikianim->~TIKIAnim();
		delete[] reinterpret_cast<char*>(tikianim);
	}
}

size_t TIKI::GetNumServerInitCommands() const
{
	if (!tikianim)
	{
		return 0;
	}

	return tikianim->num_server_initcmds;
}

const TIKICommand* TIKI::GetServerInitCommand(size_t index) const
{
	if (!tikianim)
	{
		return nullptr;
	}

	if (index >= 0 && index < tikianim->num_server_initcmds)
	{
		return &tikianim->server_initcmds[index];
	}
	else
	{
		return nullptr;
	}
}

size_t TIKI::GetNumClientInitCommands() const
{
	if (!tikianim)
	{
		return 0;
	}

	return tikianim->num_client_initcmds;
}

const TIKICommand* TIKI::GetClientInitCommand(size_t index) const
{
	if (!tikianim)
	{
		return nullptr;
	}

	if (index >= 0 && index < tikianim->num_client_initcmds)
	{
		return &tikianim->client_initcmds[index];
	}
	else
	{
		return nullptr;
	}
}

size_t TIKI::GetNumMeshes() const
{
	return meshes.size();
}

SkeletonPtr TIKI::GetMesh(size_t index) const
{
	assert(index >= 0 && index < meshes.size());
	if (index < 0 || index >= meshes.size()) {
		return NULL;
	}

	return meshes[index];
}

std::vector<SharedPtr<Skeleton>>& TIKI::getMeshes()
{
	return meshes;
}

size_t TIKI::GetNumSurfaces() const
{
	return surfaces.size();
}

const TIKISurface *TIKI::GetSurface(size_t index) const
{
	assert(index >= 0 && index < surfaces.size());
	if (index < 0 || index >= surfaces.size())
	{
		return NULL;
	}

	return &surfaces[index];
}

std::vector<TIKISurface>& TIKI::getSurfaces()
{
	return surfaces;
}

float TIKI::GetScale() const
{
	return loadScale;
}

const QuakedSection& TIKI::GetQuakedSection() const
{
	return quakedSection;
}

QuakedSection& TIKI::GetQuakedSection()
{
	return quakedSection;
}

void TIKIReader::SetupIndividualSurface(TIKISurface* surf, const char* name, const dloadsurface_t& loadsurf)
{
	for (auto it = loadsurf.shader.begin(); it != loadsurf.shader.end(); ++it)
	{
		surf->name = name;
		surf->shaders.push_back(*it);
	}
	/*
	const size_t shaderCount = loadsurf.shader.size();
	for (size_t i = 0; i < shaderCount; i++)
	{
		surf->name = name;
		surf->shaders.push_back(loadsurf.shader[i]);
	}
	*/

	surf->flags = loadsurf.flags;
	surf->damageMultiplier = loadsurf.damage_multiplier;
}

const TIKIAnim* TIKI::getTikiAnim() const
{
	return tikianim;
}

void TIKI::setLoadScale(float loadScaleValue)
{
	loadScale = loadScaleValue;
}

void TIKI::setLodScale(float lodScaleValue)
{
	lodScale = lodScaleValue;
}

void TIKI::setLodBias(float lodBiasValue)
{
	lodBias = lodBiasValue;
}

void TIKI::setLightOffset(const_vec3r_t lightOffsetValue)
{
	lightOffset[0] = lightOffsetValue[0];
	lightOffset[1] = lightOffsetValue[1];
	lightOffset[2] = lightOffsetValue[2];
}

void TIKI::setLoadOrigin(const_vec3r_t loadOriginValue)
{
	loadOrigin[0] = loadOriginValue[0];
	loadOrigin[1] = loadOriginValue[1];
	loadOrigin[2] = loadOriginValue[2];

}

void TIKI::setRadius(float radiusValue)
{
	radius = radiusValue;
}

TIKICommand::TIKICommand() noexcept
	: frame_num(0)
	, num_args(0)
	, args(nullptr)
{

}

size_t TIKICommand::getNumArgs() const
{
	return num_args;
}

const char* TIKICommand::getArg(uintptr_t index) const
{
	return args[index];
}

frameInt_t TIKICommand::getFrameNumber() const
{
	return frame_num;
}

TIKIAnimDef::TIKIAnimDef() noexcept
	: alias(nullptr)
	, client_cmds(nullptr)
	, server_cmds(nullptr)
	, num_client_cmds(0)
	, num_server_cmds(0)
	, weight(0.f)
	, blendtime(0.f)
	, flags(0)
{

}

const char* TIKIAnimDef::getAlias() const
{
	return alias;
}

size_t TIKIAnimDef::getNumClientCommands() const
{
	return num_client_cmds;
}

const TIKICommand* TIKIAnimDef::getClientCommand(uintptr_t index) const
{
	return &client_cmds[index];
}

size_t TIKIAnimDef::getNumServerCommands() const
{
	return num_server_cmds;
}

const TIKICommand* TIKIAnimDef::getServerCommand(uintptr_t index) const
{
	return &server_cmds[index];
}

const SharedPtr<SkeletonAnimation>& TIKIAnimDef::getAnimData() const
{
	return animData;
}

float TIKIAnimDef::getWeight() const
{
	return weight;
}

float TIKIAnimDef::getBlendTime() const
{
	return blendtime;
}

uint32_t TIKIAnimDef::getFlags() const
{
	return flags;
}

TIKIAnim::TIKIAnim() noexcept
	: animdefs(nullptr)
	, client_initcmds(nullptr)
	, server_initcmds(nullptr)
	, headmodels(nullptr)
	, headskins(nullptr)
	, num_headmodels(0)
	, num_headskins(0)
	, num_client_initcmds(0)
	, num_server_initcmds(0)
	, num_animdefs(0)
	, mins{ 0 }
	, maxs{ 0 }
	, bIsCharacter(false)
{

}

size_t TIKIAnim::getNumAnimDefs() const
{
	return num_animdefs;
}

const TIKIAnimDef* TIKIAnim::getAnimDef(uintptr_t index) const
{
	return &animdefs[index];
}

size_t TIKIAnim::getNumClientInitCommands() const
{
	return num_client_initcmds;
}

const TIKICommand* TIKIAnim::getClientInitCommands(uintptr_t index) const
{
	return &client_initcmds[index];
}

size_t TIKIAnim::getNumServerInitCommands() const
{
	return num_server_initcmds;
}

const TIKICommand* TIKIAnim::getServerInitCommands(uintptr_t index) const
{
	return &server_initcmds[index];
}

size_t TIKIAnim::getNumHeadModels() const
{
	return num_headmodels;
}

const char* TIKIAnim::getHeadModel(uintptr_t index) const
{
	return headmodels[index];
}

size_t TIKIAnim::getNumHeadSkins() const
{
	return num_headskins;
}

const char* TIKIAnim::getHeadSkin(uintptr_t index) const
{
	return headskins[index];
}

void TIKIAnim::getMins(vec3r_t out) const
{
	out[0] = mins[0];
	out[1] = mins[1];
	out[2] = mins[2];
}

void TIKIAnim::getMaxs(vec3r_t out) const
{
	out[0] = maxs[0];
	out[1] = maxs[1];
	out[2] = maxs[2];
}

bool TIKIAnim::isCharacter() const
{
	return bIsCharacter;
}
