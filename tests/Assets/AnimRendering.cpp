#include <MOHPC/Assets/Formats/TIKI.h>
#include <MOHPC/Assets/Managers/AssetManager.h>
#include <MOHPC/Assets/Managers/SkeletorManager.h>
#include <MOHPC/Utility/ModelRenderer.h>

#include "Common/Common.h"

#include <cmath>
#include <cassert>

bool isOffsetCorrect(float offset)
{
	return offset > -1000.f && offset < 1000.f;
}

int main(int argc, const char* argv[])
{
	InitCommon(argc, argv);

	const MOHPC::AssetManagerPtr AM = AssetLoad(GetGamePathFromCommandLine());

	MOHPC::SkeletonAnimationPtr Animation = AM->readAsset<MOHPC::SkeletonAnimationReader>("models/human/animation/deaths/death_chest.skc");
	MOHPC::TIKIPtr Tiki = AM->readAsset<MOHPC::TIKIReader>("models/player/american_army.tik");
	MOHPC::SkeletonPtr TikiArms = AM->readAsset<MOHPC::SkeletonReader>("models/player/us_army/USarmyplyr.skd");

	if (Tiki)
	{
		MOHPC::ModelRendererPtr ModelRenderer = MOHPC::ModelRenderer::create(AM);
		ModelRenderer->AddModel(Tiki.get());
		ModelRenderer->AddModel(TikiArms);

		for (size_t i = 0; i < Animation->GetNumFrames(); i++)
		{
			ModelRenderer->SetActionPose(Animation, 0, i);
			ModelRenderer->BuildBonesTransform();
			ModelRenderer->BuildRenderData();

			const size_t numSurfs = ModelRenderer->GetNumSurfaces();
			for (size_t j = 0; j < numSurfs; ++j)
			{
				const ModelSurface* surf = ModelRenderer->GetSurface(j);
				for (auto it = surf->vertices.begin(); it != surf->vertices.end(); ++it)
				{
					const ModelVertice& vert = *it;
					assert(isOffsetCorrect(vert.xyz[0]));
					assert(isOffsetCorrect(vert.xyz[1]));
					assert(isOffsetCorrect(vert.xyz[2]));
					assert(isOffsetCorrect(vert.normal[0]));
					assert(isOffsetCorrect(vert.normal[1]));
					assert(isOffsetCorrect(vert.normal[2]));
					assert(isOffsetCorrect(vert.st[0]));
					assert(isOffsetCorrect(vert.st[1]));

					for (auto jt = vert.weights.begin(); jt != vert.weights.end(); ++jt)
					{
						const ModelWeight& weight = *jt;
						assert(isOffsetCorrect(weight.boneWeight));
						assert(isOffsetCorrect(weight.offset[0]));
						assert(isOffsetCorrect(weight.offset[1]));
						assert(isOffsetCorrect(weight.offset[2]));
					}
				}
			}
		}
	}
}
