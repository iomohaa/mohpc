#include <MOHPC/Formats/TIKI.h>
#include <MOHPC/Managers/AssetManager.h>
#include <MOHPC/Managers/SkeletorManager.h>
#include <MOHPC/Utilities/ModelRenderer.h>

void TestAnimRendering(MOHPC::AssetManager& AM)
{
	MOHPC::SkeletonAnimationPtr Animation = AM.LoadAsset<MOHPC::SkeletonAnimation>("/models/human/animation/deaths/death_chest.skc");
	MOHPC::TIKIPtr Tiki = AM.LoadAsset<MOHPC::TIKI>("/models/player/american_army.tik");
	MOHPC::SkeletonPtr TikiArms = AM.LoadAsset<MOHPC::Skeleton>("/models/player/us_army/USarmyplyr.skd");

	if (Tiki)
	{
		MOHPC::ModelRenderer ModelRenderer;
		ModelRenderer.InitAssetManager(&AM);
		ModelRenderer.AddModel(Tiki.get());
		ModelRenderer.AddModel(TikiArms.get());

		for (size_t i = 0; i < Animation->GetNumFrames(); i++)
		{
			ModelRenderer.SetActionPose(Animation.get(), 0, i);
			ModelRenderer.BuildBonesTransform();
			ModelRenderer.BuildRenderData();
		}
	}
}