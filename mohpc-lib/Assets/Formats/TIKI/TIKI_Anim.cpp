#include <Shared.h>
#include "TIKI_Private.h"

#include <cstdlib>
#include <cassert>

using namespace MOHPC;

static float randweight()
{
	return ((float)(rand() & 0x7fff)) / ((float)0x8000);
}

size_t TIKI::GetNumAnimations() const
{
	return tikianim ? tikianim->num_animdefs : 0;
}

SkeletonAnimationPtr TIKI::GetAnimation(size_t num) const
{
	const TIKIAnimDef* panimdef = GetAnimDef(num);
	if (panimdef)
	{
		return panimdef->animData;
	}

	return nullptr;
}

const TIKIAnimDef* TIKI::GetAnimDef(size_t num) const
{
	assert(tikianim);
	assert(num >= 0 && num < tikianim->num_animdefs);
	if (!tikianim || num < 0 || num >= tikianim->num_animdefs)
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
	const TIKIAnimDef* panimdef = GetAnimDefByName(name);
	if (panimdef)
	{
		return panimdef->animData;
	}

	return nullptr;
}

const TIKIAnimDef* TIKI::GetAnimDefByName(const char *name) const
{
	intptr_t iTop;
	intptr_t iBottom;
	intptr_t iMiddle;
	intptr_t iComp;
	TIKIAnimDef *panimdef;
	std::vector<float> fAnimWeights;
	float fWeight;
	float fTotalWeight;
	intptr_t i, k;

	const intptr_t numAnims = tikianim->num_animdefs;

	iBottom = 0;
	iTop = numAnims - 1;

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
			iComp = strHelpers::icmp(panimdef->alias, name);
		}

		if (!iComp)
		{
			if (!(panimdef->flags & TAF::RANDOM))
			{
				return &tikianim->animdefs[iMiddle];
			}

			for (i = iMiddle; i > 0; i--)
			{
				if (strHelpers::icmp(panimdef->alias, tikianim->animdefs[i - 1].alias))
				{
					break;
				}
			}

			k = i;

			for (iMiddle++; iMiddle < numAnims; iMiddle++)
			{
				if (strHelpers::icmp(panimdef->alias, tikianim->animdefs[iMiddle].alias))
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

				if (panimdef->flags & TAF::AUTOSTEPS_DOG)
				{
					fAnimWeights.push_back(0.0f);
					panimdef->flags &= ~TAF::AUTOSTEPS_DOG;
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
			if (panimdef && panimdef->flags & TAF::NOREPEAT)
			{
				panimdef->flags |= TAF::AUTOSTEPS_DOG;
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

const TIKIAnimDef* TIKI::GetRandomAnimation(const char *name) const
{
	std::vector<TIKIAnimDef*> anims;
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
			TIKIAnimDef *panimdef = anims[i];
			if (weight < panimdef->weight)
			{
				return panimdef;
			}

			weight -= panimdef->weight;
		}
	}

	return nullptr;
}

void TIKI::GetAllAnimations(const char* name, std::vector<TIKIAnimDef*>& out) const
{
	TIKIAnimDef *panimdef;
	std::vector<TIKIAnimDef*> anims;
	size_t len;
	int diff;

	len = strHelpers::len(name);
	if (!len)
	{
		return;
	}

	const size_t numAnims = tikianim->num_animdefs;
	for (size_t i = 0; i < numAnims; i++)
	{
		panimdef = &tikianim->animdefs[i];
		diff = strHelpers::icmpn(panimdef->alias, name, len);
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
