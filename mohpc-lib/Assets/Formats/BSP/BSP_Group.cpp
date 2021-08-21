#include <MOHPC/Assets/Formats/BSP_Group.h>
#include <MOHPC/Assets/Formats/BSP.h>
#include "../../../Common/VectorPrivate.h"

using namespace MOHPC;

struct BSPData::Patch
{
	Patch* parent;
	const BSPData::Surface* surface;
	BSPData::GroupedSurfaces* groupedSurfaces;
	vec3_t bounds[2];

	Patch(const BSPData::Surface* inSurface)
		: parent(nullptr)
		, surface(inSurface)
		, groupedSurfaces(nullptr)
	{
		const size_t numVerts = surface->GetNumVertices();
		for (size_t i = 0; i < numVerts; ++i)
		{
			AddPointToBounds(surface->GetVertice(i)->xyz, bounds[0], bounds[1]);
		}
	}

	Patch* GetRoot() const
	{
		Patch* p = const_cast<Patch*>(this);
		while (p->parent)
		{
			p = p->parent;
		}

		return p;
	}

	bool HasParent(const Patch* other) const
	{
		Patch* p = parent;
		for (Patch* p = parent; p; p = p->parent)
		{
			if (p == other)
			{
				return true;
			}
		}

		return false;
	}

	bool IsTouching(const Patch* other) const
	{
		if (other->bounds[0][0] > bounds[1][0])
		{
			return false;
		}
		if (other->bounds[0][1] > bounds[1][1])
		{
			return false;
		}
		if (other->bounds[0][2] > bounds[1][2])
		{
			return false;
		}
		if (other->bounds[1][0] < bounds[0][0])
		{
			return false;
		}
		if (other->bounds[1][1] < bounds[0][1])
		{
			return false;
		}
		if (other->bounds[1][2] < bounds[0][2])
		{
			return false;
		}

		return true;
	}
};

static bool BrushIsTouching(const BSPData::Brush* b1, const BSPData::Brush* b2)
{
	if (b1->bounds[0][0] > b2->bounds[1][0])
	{
		return false;
	}
	if (b1->bounds[0][1] > b2->bounds[1][1])
	{
		return false;
	}
	if (b1->bounds[0][2] > b2->bounds[1][2])
	{
		return false;
	}
	if (b1->bounds[1][0] < b2->bounds[0][0])
	{
		return false;
	}
	if (b1->bounds[1][1] < b2->bounds[0][1])
	{
		return false;
	}
	if (b1->bounds[1][2] < b2->bounds[0][2])
	{
		return false;
	}

	return true;
}

MOHPC_OBJECT_DEFINITION(BSPGroup);

BSPGroup::BSPGroup()
{
}

BSPGroup::~BSPGroup()
{
	for (size_t i = 0; i < groupedSurfaces.size(); i++)
	{
		delete groupedSurfaces[i];
	}
}

size_t BSPGroup::getNumGroupedSurfaces() const
{
	return groupedSurfaces.size();
}

const BSPData::GroupedSurfaces* BSPGroup::getGroupedSurface(size_t surfsGroupNum) const
{
	return groupedSurfaces.at(surfsGroupNum);
}

size_t BSPGroup::getNumBrushData() const
{
	return brushDataList.size();
}

const BSPData::BrushGroupData* BSPGroup::getBrushData(size_t brushDataNum) const
{
	return &brushDataList[brushDataNum];
}

void BSPGroup::groupSurfaces(const BSP& bsp)
{
	mapBrushes(bsp, groupedSurfaces);
}

void BSPGroup::connectBrushes(const BSP& bsp, std::vector<BSPData::BrushGroupData>& list)
{
	const size_t numBrushes = bsp.GetNumBrushes();
	const size_t startList = brushDataList.size();
	brushDataList.resize(startList + numBrushes);

	size_t numParentedBrushes = 0;
	// Connects brushes by finding touching brushes that share at least one same shader
	for (size_t b = 0; b < numBrushes; b++)
	{
		const BSPData::Brush* brush = bsp.GetBrush(b);
		BSPData::BrushGroupData& brushData = list[startList + b];
		for (size_t i = 0; i < numBrushes; i++)
		{
			const BSPData::Brush* brush2 = bsp.GetBrush(i);
			BSPData::BrushGroupData& brushData2 = list[startList + i];
			if (brush2 == brush || brushData2.parent)
			{
				// ignore parented brushes
				continue;
			}

			const BSPData::Brush* parent = brush;
			const BSPData::BrushGroupData* parentData = &brushData;

			// Checks for infinite parenting
			bool bInfiniteParent = false;
			do
			{
				if (parent == brush2)
				{
					bInfiniteParent = true;
					break;
				}
				parent = parentData->parent;
				parentData = parentData->parentData;
			} while (parentData);

			if (bInfiniteParent)
			{
				// would cause an infinite loop
				continue;
			}

			// Checks if the two brushes are touching
			if (BrushIsTouching(brush, brush2))
			{
				bool bFound = false;

				// Finds at least one valid brush side that matches the parent's brush sides
				for (size_t bs = 0; bs < brush->GetNumSides(); bs++)
				{
					const BSPData::BrushSide* brushside = brush->GetSide(bs);
					if (brushside->surfaceFlags & SURF_NODRAW)
					{
						// ignore brush that don't draw
						continue;
					}

					for (size_t bs2 = 0; bs2 < brush2->GetNumSides(); bs2++)
					{
						const BSPData::BrushSide* brushside2 = brush2->GetSide(bs2);
						if (brushside2->surfaceFlags & SURF_NODRAW) {
							continue;
						}

						if (!strHelpers::icmp(brushside2->shader->shaderName.c_str(), brushside->shader->shaderName.c_str()))
						{
							// found sides with the same shader
							bFound = true;
							break;
						}
					}

					if (bFound)
					{
						break;
					}
				}

				if (bFound)
				{
					numParentedBrushes++;
					brushData2.parent = brush;
					brushData2.parentData = &brushData;
				}
			}
		}
	}
}

void BSPGroup::mapBrushes(
	const BSP& bsp,
	std::vector<BSPData::GroupedSurfaces*>& outGroups
)
{
	const size_t startList = brushDataList.size();

	connectBrushes(bsp, brushDataList);

	assert(bsp.GetNumSubmodels());
	const BSPData::Model* worldModel = bsp.GetSubmodel(0);

	// Map brushes
	size_t numMappedSurfaces = 0;
	size_t numUnmappedSurfaces = 0;
	size_t numValidBrushes = 0;
	size_t numGroupedSurfaces = 0;
	bool bBrushHasMappedSurface = false;

	const size_t numSurfaces = worldModel->numSurfaces;
	const size_t numTerrainSurfaces = bsp.GetNumTerrainSurfaces();
	const size_t numTotalSurfaces = numSurfaces + numTerrainSurfaces;
	bool* mappedSurfaces = new bool[numSurfaces]();

	const size_t numBrushes = bsp.GetNumBrushes();
	std::unordered_map<const BSPData::Brush*, BSPData::GroupedSurfaces*> brushToSurfaces(numBrushes);
	outGroups.reserve(numTotalSurfaces);

	// Group surfaces with brushes
	for (size_t b = 0; b < numBrushes; b++)
	{
		const BSPData::Brush* brush = bsp.GetBrush(b);
		BSPData::BrushGroupData& brushData = brushDataList[startList + b];

		const BSPData::Brush* rootbrush = brush;
		const BSPData::BrushGroupData* rootBrushData = &brushData;
		while (rootBrushData->parent != NULL)
		{
			rootbrush = rootBrushData->parent;
			rootBrushData = rootBrushData->parentData;
		}

		//const str& brushname = rootbrush->name;
		const_vec3p_t mins = brush->bounds[0];
		const_vec3p_t maxs = brush->bounds[1];

		auto it = brushToSurfaces.find(rootbrush);

		for (size_t k = 0; k < numSurfaces; k++)
		{
			if (!mappedSurfaces[k])
			{
				const BSPData::Surface* surf = &worldModel->surface[k];

				if (!surf->IsPatch()
					&& surf->centroid[0] >= mins[0] && surf->centroid[0] <= maxs[0]
					&& surf->centroid[1] >= mins[1] && surf->centroid[1] <= maxs[1]
					&& surf->centroid[2] >= mins[2] && surf->centroid[2] <= maxs[2])
				{
					for (size_t s = 0; s < brush->numsides; s++)
					{
						const BSPData::BrushSide* side = &brush->sides[s];
						if (!strHelpers::icmp(side->shader->shaderName.c_str(), surf->shader->shaderName.c_str()))
						{
							brushData.surfaces.push_back(surf);

							BSPData::GroupedSurfaces* sg = nullptr;
							if (it == brushToSurfaces.end())
							{
								sg = new BSPData::GroupedSurfaces;
								sg->name = "groupedsurf" + std::to_string(numGroupedSurfaces); // std::to_string(numGroupedSurfaces);
								outGroups.push_back(sg);
								auto res = brushToSurfaces.emplace(rootbrush, sg);
								assert(res.second);
								if (res.second)
								{
									it = res.first;
								}
								numGroupedSurfaces++;
							}
							else
							{
								sg = it->second;
							}

							sg->surfaces.push_back(surf);

							if (std::find(sg->brushes.begin(), sg->brushes.end(), brush) == sg->brushes.end())
							{
								sg->brushes.push_back(brush);
							}

							mappedSurfaces[k] = true;
							numMappedSurfaces++;
							bBrushHasMappedSurface = true;
							break;
						}
					}
				}
			}
		}

		if (bBrushHasMappedSurface)
		{
			bBrushHasMappedSurface = false;
			numValidBrushes++;
		}
	}

	// Add the LOD terrain
	BSPData::GroupedSurfaces* terSg = new BSPData::GroupedSurfaces;
	terSg->name = "lod_terrain";
	terSg->surfaces.resize(numTerrainSurfaces);

	for (size_t k = 0; k < numTerrainSurfaces; k++)
	{
		const BSPData::Surface* surf = bsp.GetTerrainSurface(k);

		terSg->surfaces[k] = surf;
	}

	outGroups.push_back(terSg);

	// Gather patches
	{
		std::vector<BSPData::Patch> patchList;
		patchList.reserve(numSurfaces);

		for (size_t k = 0; k < numSurfaces; k++)
		{
			const BSPData::Surface* surf = &worldModel->surface[k];
			if (surf->IsPatch())
			{
				patchList.emplace_back(surf);
			}
		}

		const size_t numPatches = patchList.size();
		for (size_t k = 0; k < numPatches; k++)
		{
			BSPData::Patch* patch1 = &patchList[k];
			for (size_t l = 0; l < numPatches; l++)
			{
				if (l != k)
				{
					BSPData::Patch* patch2 = &patchList[l];
					if (!patch1->HasParent(patch2) && patch2->IsTouching(patch1))
					{
						patch2->parent = patch1;
					}
				}
			}
		}

		size_t numGroupedPatches = 0;
		for (size_t k = 0; k < numPatches; k++)
		{
			BSPData::Patch* patch = &patchList[k];
			BSPData::Patch* rootPatch = patch->GetRoot();

			// Get the number of surfaces
			size_t numSurfaces = 0;
			for (BSPData::Patch* p = patch; p; p = p->parent)
			{
				bool& isMappedSurface = mappedSurfaces[p->surface - worldModel->surface];
				if (!isMappedSurface)
				{
					numSurfaces++;
				}
			}

			if (numSurfaces > 0)
			{
				if (!rootPatch->groupedSurfaces)
				{
					rootPatch->groupedSurfaces = new BSPData::GroupedSurfaces;
					rootPatch->groupedSurfaces->name = "meshpatch_grouped" + std::to_string(numGroupedPatches++); // std::to_string(numGroupedPatches++);
					outGroups.push_back(rootPatch->groupedSurfaces);
				}

				BSPData::GroupedSurfaces* sg = rootPatch->groupedSurfaces;

				sg->surfaces.reserve(sg->surfaces.size() + numSurfaces);
				for (BSPData::Patch* p = patch; p; p = p->parent)
				{
					bool& isMappedSurface = mappedSurfaces[p->surface - worldModel->surface];
					if (!isMappedSurface)
					{
						sg->surfaces.push_back(p->surface);
						isMappedSurface = true;
					}
				}
			}
		}
	}

	// Group unused surfaces
	for (size_t k = 0; k < numSurfaces; k++)
	{
		if (!mappedSurfaces[k])
		{
			const BSPData::Surface* surf = &worldModel->surface[k];

			BSPData::GroupedSurfaces* sg = new BSPData::GroupedSurfaces;
			sg->name = "GroupedSurfaces_unmapped" + std::to_string(numUnmappedSurfaces); // std::to_string(numUnmappedSurfaces);
			sg->surfaces.push_back(surf);
			outGroups.push_back(sg);

			numUnmappedSurfaces++;
		}
	}

	// Calculate the origin for each group of surfaces
	for (size_t i = 0; i < outGroups.size(); i++)
	{
		BSPData::GroupedSurfaces* sg = outGroups[i];

		Vector3 avg(0, 0, 0);
		size_t numVertices = 0;

		for (size_t k = 0; k < sg->surfaces.size(); k++)
		{
			const BSPData::Surface* surf = sg->surfaces[k];

			for (size_t v = 0; v < surf->vertices.size(); v++)
			{
				const BSPData::Vertice* vert = &surf->vertices[v];

				avg += castVector(vert->xyz);

				AddPointToBounds(surf->vertices[v].xyz, sg->bounds[0], sg->bounds[1]);
			}

			numVertices += surf->vertices.size();
		}

		castVector(sg->origin) = avg / (float)numVertices;
	}

	outGroups.shrink_to_fit();
	delete[] mappedSurfaces;
}

const str& BSPData::GroupedSurfaces::GetGroupName() const
{
	return name;
}

size_t BSPData::GroupedSurfaces::GetNumSurfaces() const
{
	return surfaces.size();
}

const BSPData::Surface* BSPData::GroupedSurfaces::GetSurface(size_t index) const
{
	return surfaces.at(index);
}

const BSPData::Surface* const* BSPData::GroupedSurfaces::GetSurfaces() const
{
	return surfaces.data();
}

size_t BSPData::GroupedSurfaces::GetNumBrushes() const
{
	return brushes.size();
}

const BSPData::Brush* BSPData::GroupedSurfaces::GetBrush(size_t index) const
{
	return brushes.at(index);
}

const BSPData::Brush* const* BSPData::GroupedSurfaces::GetBrushes() const
{
	return brushes.data();
}

const_vec3p_t BSPData::GroupedSurfaces::GetMinBound() const
{
	return bounds[0];
}

const_vec3p_t BSPData::GroupedSurfaces::GetMaxBound() const
{
	return bounds[1];
}

const_vec3p_t BSPData::GroupedSurfaces::GetOrigin() const
{
	return origin;
}

BSPData::BrushGroupData::BrushGroupData()
	: parent(nullptr)
	, parentData(nullptr)
{}
