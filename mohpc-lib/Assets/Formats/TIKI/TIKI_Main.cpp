#include "TIKI_Private.h"

#include <cassert>

using namespace MOHPC;

MOHPC_OBJECT_DEFINITION(TIKI);
TIKI::TIKI(const fs::path& path)
	: Asset2(path)
{
	tikianim = NULL;
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
		delete tikianim;
	}
}

size_t TIKI::GetNumServerInitCommands() const
{
	if (!tikianim)
	{
		return 0;
	}

	return tikianim->server_initcmds.size();
}

const TIKIAnim::Command* TIKI::GetServerInitCommand(size_t index) const
{
	if (!tikianim)
	{
		return nullptr;
	}

	if (index >= 0 && index < tikianim->server_initcmds.size())
	{
		return &tikianim->server_initcmds.at(index);
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

	return tikianim->client_initcmds.size();
}

const TIKIAnim::Command* TIKI::GetClientInitCommand(size_t index) const
{
	if (!tikianim)
	{
		return nullptr;
	}

	if (index >= 0 && index < tikianim->client_initcmds.size())
	{
		return &tikianim->client_initcmds.at(index);
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

std::vector<SharedPtr<MOHPC::Skeleton>>& MOHPC::TIKI::getMeshes()
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

std::vector<MOHPC::TIKISurface>& TIKI::getSurfaces()
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

QuakedSection& MOHPC::TIKI::GetQuakedSection()
{
	return quakedSection;
}

void TIKIReader::SetupIndividualSurface(const fs::path& filename, TIKISurface* surf, const char* name, const dloadsurface_t* loadsurf)
{
	const size_t shaderCount = loadsurf->shader.size();
	for (size_t i = 0; i < shaderCount; i++)
	{
		surf->name = name;
		surf->shaders.push_back(loadsurf->shader[i]);
	}

	surf->flags = loadsurf->flags;
	surf->damageMultiplier = loadsurf->damage_multiplier;
}

TIKIAnim* MOHPC::TIKI::getTikiAnim() const
{
	return tikianim;
}

void MOHPC::TIKI::setTikiAnim(TIKIAnim* anim)
{
	tikianim = anim;
}

void MOHPC::TIKI::setLoadScale(float loadScaleValue)
{
	loadScale = loadScaleValue;
}

void MOHPC::TIKI::setLodScale(float lodScaleValue)
{
	lodScale = lodScaleValue;
}

void MOHPC::TIKI::setLodBias(float lodBiasValue)
{
	lodBias = lodBiasValue;
}

void MOHPC::TIKI::setLightOffset(const_vec3r_t lightOffsetValue)
{
	lightOffset[0] = lightOffsetValue[0];
	lightOffset[1] = lightOffsetValue[1];
	lightOffset[2] = lightOffsetValue[2];
}

void MOHPC::TIKI::setLoadOrigin(const_vec3r_t loadOriginValue)
{
	loadOrigin[0] = loadOriginValue[0];
	loadOrigin[1] = loadOriginValue[1];
	loadOrigin[2] = loadOriginValue[2];

}

void MOHPC::TIKI::setRadius(float radiusValue)
{
	radius = radiusValue;
}
