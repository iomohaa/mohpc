#include <MOHPC/Assets/Formats/TIKI.h>
#include <MOHPC/Assets/Managers/AssetManager.h>
#include <MOHPC/Files/Managers/FileManager.h>
#include <MOHPC/Utility/ModelRenderer.h>
#include "Common/Common.h"

int main(int argc, const char* argv[])
{
	const MOHPC::AssetManagerPtr AM = AssetLoad();

	mfuse::con::Container<str> ExtensionsStd;
	ExtensionsStd.AddObject("wav");

	MOHPC::FileEntryList FilesStd = AM->GetFileManager()->ListFilteredFiles("/", ExtensionsStd, true, false);

	MOHPC::TIKIPtr Tiki = AM->LoadAsset<MOHPC::TIKI>("/models/static/toilet.tik");
	if (Tiki)
	{
		MOHPC::ModelRendererPtr ModelRenderer = MOHPC::ModelRenderer::create(AM);
		ModelRenderer->AddModel(Tiki.get());
		ModelRenderer->BuildBonesTransform();
		ModelRenderer->BuildRenderData();
	}

	Tiki = AM->LoadAsset<MOHPC::TIKI>("/models/items/item_50_healthbox.tik");

	MOHPC::SkeletonAnimationPtr AnimPtr = AM->LoadAsset<MOHPC::SkeletonAnimation>("/models/human/animation/viewmodel/walk_player_rifle.skc");

	//auto Asset = AM->LoadAsset<MOHPC::TIKI>("/models/weapons/m1_garand.tik");
	//Asset = AM->LoadAsset<MOHPC::TIKI>("/models/static/bush_full.tik");
	//Asset = AM->LoadAsset<MOHPC::TIKI>("/models/human/allied_airborne_soldier.tik");
	auto Asset = AM->LoadAsset<MOHPC::TIKI>("/models/player/american_Army.tik");
	if (Asset)
	{
		MOHPC::ModelRendererPtr ModelRenderer = MOHPC::ModelRenderer::create(AM);
		ModelRenderer->AddModel(Asset.get());
		ModelRenderer->SetActionPose(AnimPtr, 0, 0, 1.f);
		ModelRenderer->BuildBonesTransform();
		ModelRenderer->BuildRenderData();
	}
}
