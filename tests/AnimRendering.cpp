#include <MOHPC/Formats/TIKI.h>
#include <MOHPC/Managers/AssetManager.h>
#include <MOHPC/Managers/SkeletorManager.h>
#include <MOHPC/Utilities/ModelRenderer.h>
#include "UnitTest.h"

class CAnimRenderingTest : public IUnitTest, public TAutoInst<CAnimRenderingTest>
{
public:
	virtual const char* name() override
	{
		return "Anim rendering";
	}

	virtual void run(const MOHPC::AssetManagerPtr& AM) override
	{
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
};