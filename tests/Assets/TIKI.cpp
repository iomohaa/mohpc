#include <MOHPC/Assets/Formats/TIKI.h>
#include <MOHPC/Assets/Managers/AssetManager.h>
#include <MOHPC/Files/Managers/IFileManager.h>
#include <MOHPC/Utility/ModelRenderer.h>
#include "Common/Common.h"

static constexpr char MOHPC_LOG_NAMESPACE[] = "test_tiki";

int main(int argc, const char* argv[])
{
	InitCommon(argc, argv);

	std::vector<int> test;
	test.push_back(1);
	test.push_back(2);
	test.push_back(3);

	const MOHPC::AssetManagerPtr AM = AssetLoad(GetGamePathFromCommandLine());
	const MOHPC::IFileManagerPtr FM = AM->GetFileManager();

	// preload the shader manager
	AM->getManager<MOHPC::ShaderManager>();

	MOHPC::TIKIPtr Tiki;
	Tiki = AM->readAsset<MOHPC::TIKIReader>("models/player/_american_Army_cold.tik");
	Tiki = AM->readAsset<MOHPC::TIKIReader>("models/human/1st-ranger_captain.tik");
	Tiki = AM->readAsset<MOHPC::TIKIReader>("/models/static/toilet.tik");
	if (Tiki)
	{
		MOHPC::ModelRendererPtr ModelRenderer = MOHPC::ModelRenderer::create(AM);
		ModelRenderer->AddModel(Tiki.get());
		ModelRenderer->BuildBonesTransform();
		ModelRenderer->BuildRenderData();
	}

	Tiki = AM->readAsset<MOHPC::TIKIReader>("/models/items/item_50_healthbox.tik");

	MOHPC::SkeletonAnimationPtr AnimPtr = AM->readAsset<MOHPC::SkeletonAnimationReader>("/models/human/animation/viewmodel/walk_player_rifle.skc");

	Tiki = AM->readAsset<MOHPC::TIKIReader>("/models/weapons/m1_garand.tik");
	Tiki = AM->readAsset<MOHPC::TIKIReader>("/models/static/bush_full.tik");
	Tiki = AM->readAsset<MOHPC::TIKIReader>("/models/human/allied_airborne_soldier.tik");
	Tiki = AM->readAsset<MOHPC::TIKIReader>("/models/player/american_Army.tik");
	if (Tiki)
	{
		MOHPC::ModelRendererPtr ModelRenderer = MOHPC::ModelRenderer::create(AM);
		ModelRenderer->AddModel(Tiki.get());
		ModelRenderer->SetActionPose(AnimPtr, 0, 0, 1.f);
		ModelRenderer->BuildBonesTransform();
		ModelRenderer->BuildRenderData();
	}

	MOHPC::FileEntryList tikiList = FM->ListFilteredFiles("/", "tik", true);

	const size_t numFiles = tikiList.GetNumFiles();

	std::vector<MOHPC::TIKIPtr> tikiAssetList;
	tikiAssetList.reserve(numFiles);

	for (size_t i = 0; i < numFiles; ++i)
	{
		const MOHPC::FileEntry* entry = tikiList.GetFileEntry(i);

		if (!entry->IsDirectory())
		{
			try
			{
				MOHPC_LOG(Info, "loading '%s'", entry->getName().generic_string().c_str());
				tikiAssetList.emplace_back(AM->readAsset<MOHPC::TIKIReader>(entry->getName()));
			}
			catch (const std::exception& exc)
			{
				MOHPC_LOG(Error, "exception loading asset: '%s'", exc.what());
			}
		}
	}
}
