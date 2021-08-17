#include <MOHPC/Assets/Formats/TIKI.h>
#include <MOHPC/Assets/Managers/AssetManager.h>
#include <MOHPC/Files/Managers/FileManager.h>
#include <MOHPC/Utility/ModelRenderer.h>
#include "Common/Common.h"

int main(int argc, const char* argv[])
{
	InitCommon();

	const MOHPC::AssetManagerPtr AM = AssetLoad(GetGamePathFromCommandLine(argc, argv));

	ExtensionList ExtensionsStd;
	ExtensionsStd.push_back("wav");

	MOHPC::FileEntryList FilesStd = AM->GetFileManager()->ListFilteredFiles("/", ExtensionsStd, true, false);

	MOHPC::TIKIPtr Tiki = AM->readAsset<MOHPC::TIKIReader>("/models/static/toilet.tik");
	if (Tiki)
	{
		MOHPC::ModelRendererPtr ModelRenderer = MOHPC::ModelRenderer::create(AM);
		ModelRenderer->AddModel(Tiki.get());
		ModelRenderer->BuildBonesTransform();
		ModelRenderer->BuildRenderData();
	}

	Tiki = AM->readAsset<MOHPC::TIKIReader>("/models/items/item_50_healthbox.tik");

	MOHPC::SkeletonAnimationPtr AnimPtr = AM->readAsset<MOHPC::SkeletonAnimationReader>("/models/human/animation/viewmodel/walk_player_rifle.skc");

	MOHPC::TIKIPtr Asset = AM->readAsset<MOHPC::TIKIReader>("/models/weapons/m1_garand.tik");
	Asset = AM->readAsset<MOHPC::TIKIReader>("/models/static/bush_full.tik");
	Asset = AM->readAsset<MOHPC::TIKIReader>("/models/human/allied_airborne_soldier.tik");
	Asset = AM->readAsset<MOHPC::TIKIReader>("/models/player/american_Army.tik");
	if (Asset)
	{
		MOHPC::ModelRendererPtr ModelRenderer = MOHPC::ModelRenderer::create(AM);
		ModelRenderer->AddModel(Asset.get());
		ModelRenderer->SetActionPose(AnimPtr, 0, 0, 1.f);
		ModelRenderer->BuildBonesTransform();
		ModelRenderer->BuildRenderData();
	}
}
