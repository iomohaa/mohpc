#include <MOHPC/Assets/Formats/TIKI.h>
#include <MOHPC/Assets/Managers/AssetManager.h>
#include <MOHPC/Utility/Managers/SkeletorManager.h>
#include <MOHPC/Utility/ModelRenderer.h>
#include "Common/Common.h"

int main(int argc, const char* argv[])
{
	const MOHPC::AssetManagerPtr AM = AssetLoad();

	MOHPC::SkeletonAnimationPtr Animation = AM->LoadAsset<MOHPC::SkeletonAnimation>("/models/human/animation/deaths/death_chest.skc");
	MOHPC::TIKIPtr Tiki = AM->LoadAsset<MOHPC::TIKI>("/models/player/american_army.tik");
	MOHPC::SkeletonPtr TikiArms = AM->LoadAsset<MOHPC::Skeleton>("/models/player/us_army/USarmyplyr.skd");

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
		}
	}
}
