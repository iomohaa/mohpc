#include "TIKI_Private.h"

using namespace MOHPC;

CLASS_DEFINITION(TIKI);
TIKI::TIKI()
{
	tikianim = NULL;
	loadScale = 0.f;
	lodScale = 0.f;
	lodBias = 0.f;
	lightOffset = Vector(0, 0, 0);
	loadOrigin = Vector(0, 0, 0);
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

size_t TIKI::GetNumSurfaces() const
{
	return surfaces.size();
}

const TIKI::TIKISurface *TIKI::GetSurface(size_t index) const
{
	assert(index >= 0 && index < surfaces.size());
	if (index < 0 || index >= surfaces.size())
	{
		return NULL;
	}

	return &surfaces[index];
}

void TIKI::SetupIndividualSurface(const char *filename, TIKISurface* surf, const char *name, const dloadsurface_t *loadsurf)
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

float TIKI::GetScale() const
{
	return loadScale;
}

const QuakedSection& TIKI::GetQuakedSection() const
{
	return quakedSection;
}
