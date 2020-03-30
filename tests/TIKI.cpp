#include <MOHPC/Formats/TIKI.h>
#include <MOHPC/Managers/AssetManager.h>
#include <MOHPC/Utilities/ModelRenderer.h>

void TestTiki(MOHPC::AssetManager& AM)
{
	auto Asset = AM.LoadAsset<MOHPC::TIKI>("/models/weapons/m1_garand.tik");
	Asset = AM.LoadAsset<MOHPC::TIKI>("/models/static/bush_full.tik");
	Asset = AM.LoadAsset<MOHPC::TIKI>("/models/human/allied_airborne_soldier.tik");

	MOHPC::TIKIPtr Tiki = AM.LoadAsset<MOHPC::TIKI>("/models/static/toilet.tik");
	if (Tiki)
	{
		MOHPC::ModelRenderer ModelRenderer;
		ModelRenderer.InitAssetManager(&AM);
		ModelRenderer.AddModel(Tiki.get());
		ModelRenderer.BuildBonesTransform();
		ModelRenderer.BuildRenderData();
	}
}
