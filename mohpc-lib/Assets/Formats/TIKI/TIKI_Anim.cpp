#include <Shared.h>
#include "TIKI_Private.h"

#include <cassert>

using namespace MOHPC;

static float randweight()
{
	return ((float)(rand() & 0x7fff)) / ((float)0x8000);
}

#ifdef _WIN32
static int AnimCompareFunc(void *context, const void *a, const void *b)
#else
static int AnimCompareFunc(const void *a, const void *b, void *context)
#endif
{
	TIKI::dloaddef_t *ld = (TIKI::dloaddef_t*)context;
	return stricmp(ld->loadanims[*(int *)a].alias.c_str(), ld->loadanims[*(int *)b].alias.c_str());
}

void TIKI::GetAnimOrder(const dloaddef_t *ld, std::vector<size_t>& order) const
{
	for (size_t i = 0; i < ld->loadanims.size(); i++)
	{
		order.push_back(i);
	}

	qsort2(&order[0], order.size(), sizeof(size_t), AnimCompareFunc, (void*)ld);
}

size_t TIKI::GetNumAnimations() const
{
	return tikianim ? tikianim->animdefs.size() : 0;
}

SkeletonAnimationPtr TIKI::GetAnimation(size_t num) const
{
	const TIKIAnim::AnimDef* panimdef = GetAnimDef(num);
	if (panimdef)
	{
		return panimdef->animData;
	}

	return nullptr;
}

const TIKIAnim::AnimDef* TIKI::GetAnimDef(size_t num) const
{
	assert(tikianim);
	assert(num >= 0 && num < tikianim->animdefs.size());
	if (!tikianim || num < 0 || num >= tikianim->animdefs.size())
	{
		return NULL;
	}
	else
	{
		return &tikianim->animdefs[num];
	}
}

SkeletonAnimationPtr TIKI::GetAnimationByName(const char *name) const
{
	const TIKIAnim::AnimDef* panimdef = GetAnimDefByName(name);
	if (panimdef)
	{
		return panimdef->animData;
	}

	return nullptr;
}

const TIKIAnim::AnimDef* TIKI::GetAnimDefByName(const char *name) const
{
	intptr_t iTop;
	intptr_t iBottom;
	intptr_t iMiddle;
	intptr_t iComp;
	TIKIAnim::AnimDef *panimdef;
	std::vector<float> fAnimWeights;
	float fWeight;
	float fTotalWeight;
	intptr_t i, k;

	const intptr_t numAnims = tikianim->animdefs.size();

	iBottom = 0;
	iTop = tikianim->animdefs.size() - 1;

	while (iBottom <= iTop)
	{
		iMiddle = (iBottom + iTop) / 2;

		panimdef = &tikianim->animdefs[iMiddle];
		if (!panimdef)
		{
			iComp = -1;
		}
		else
		{
			iComp = stricmp(panimdef->alias.c_str(), name);
		}

		if (!iComp)
		{
			if (!(panimdef->flags & TAF_RANDOM))
			{
				return &tikianim->animdefs[iMiddle];
			}

			for (i = iMiddle; i > 0; i--)
			{
				if (stricmp(panimdef->alias.c_str(), tikianim->animdefs[i - 1].alias.c_str()))
				{
					break;
				}
			}

			k = i;

			for (iMiddle++; iMiddle < numAnims; iMiddle++)
			{
				if (stricmp(panimdef->alias.c_str(), tikianim->animdefs[iMiddle].alias.c_str()))
				{
					break;
				}
			}

			fTotalWeight = 0.0f;

			for (; i < iMiddle; i++)
			{
				panimdef = &tikianim->animdefs[i];
				if (!panimdef)
				{
					continue;
				}

				if (panimdef->flags & TAF_AUTOSTEPS_DOG)
				{
					fAnimWeights.push_back(0.0f);
					panimdef->flags &= ~TAF_AUTOSTEPS_DOG;
				}
				else
				{
					fAnimWeights.push_back(panimdef->weight);
					fTotalWeight += panimdef->weight;
				}
			}

			fWeight = rand() * fTotalWeight;
			const intptr_t animCount = fAnimWeights.size();
			for (i = 0; i < animCount; i++)
			{
				if (fWeight < fAnimWeights[i])
				{
					break;
				}

				fWeight -= fAnimWeights[i];
			}


			iMiddle = i + k;
			panimdef = &tikianim->animdefs[iMiddle];
			if (panimdef && panimdef->flags & TAF_NOREPEAT)
			{
				panimdef->flags |= TAF_AUTOSTEPS_DOG;
			}

			return &tikianim->animdefs[iMiddle];
		}

		if (iComp > 0)
		{
			iTop = iMiddle - 1;
		}
		else
		{
			iBottom = iMiddle + 1;
		}
	}

	return nullptr;
}

const TIKIAnim::AnimDef* TIKI::GetRandomAnimation(const char *name) const
{
	std::vector<TIKIAnim::AnimDef*> anims;
	GetAllAnimations(name, anims);

	// animation name found
	if (anims.size())
	{
		float totalweight = 0.f;

		const size_t numAnims = anims.size();
		for (size_t i = 0; i < numAnims; i++)
		{
			totalweight += anims[i]->weight;
		}

		// find a random animation based on the weight
		float weight = randweight() * totalweight;
		for (size_t i = 0; i < numAnims; i++)
		{
			TIKIAnim::AnimDef *panimdef = anims[i];
			if (weight < panimdef->weight)
			{
				return panimdef;
			}

			weight -= panimdef->weight;
		}
	}

	return nullptr;
}

void TIKI::GetAllAnimations(const char* name, std::vector<TIKIAnim::AnimDef*>& out) const
{
	TIKIAnim::AnimDef *panimdef;
	std::vector<TIKIAnim::AnimDef*> anims;
	size_t len;
	int diff;

	len = strlen(name);
	if (!len)
	{
		return;
	}

	const size_t numAnims = tikianim->animdefs.size();
	for (size_t i = 0; i < numAnims; i++)
	{
		panimdef = &tikianim->animdefs[i];
		diff = strnicmp(panimdef->alias.c_str(), name, len);
		if (diff || panimdef->alias[len] == '_')
		{
			if (diff > 0)
			{
				break;
			}
		}
		else
		{
			out.push_back(panimdef);
		}
	}
}

bool TIKI::IsStaticModel() const
{
	if (GetNumAnimations())
	{
		for (size_t animNum = 0; animNum < GetNumAnimations(); ++animNum)
		{
			const SkeletonAnimationPtr Animation = GetAnimation(animNum);

			bool bFoundPrev = false;

			// Skip already compared animation
			for (size_t foundAnimNum = 0; foundAnimNum < animNum; ++foundAnimNum)
			{
				if (Animation == GetAnimation(foundAnimNum))
				{
					bFoundPrev = true;
				}
			}

			if (bFoundPrev)
			{
				continue;
			}

			if (Animation->IsDynamic())
			{
				return false;
			}
		}
	}

	return true;
}
