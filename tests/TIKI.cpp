#include <MOHPC/Formats/TIKI.h>
#include <MOHPC/Managers/AssetManager.h>
#include <MOHPC/Managers/FileManager.h>
#include <MOHPC/Utilities/ModelRenderer.h>
#include "UnitTest.h"

class CTikiTest : public IUnitTest, public TAutoInst<CTikiTest>
{
public:
	virtual const char* name() override
	{
		return "TIKI";
	}

	virtual unsigned int priority() override
	{ 
		return 0;
	}

	virtual void run(const MOHPC::AssetManagerPtr& AM) override
	{
		MOHPC::Container<MOHPC::str> ExtensionsStd;
		ExtensionsStd.AddObject("wav");

		MOHPC::FileEntryList FilesStd = AM->GetFileManager()->ListFilteredFiles("/", ExtensionsStd, true, false);

		auto Asset = AM->LoadAsset<MOHPC::TIKI>("/models/weapons/m1_garand.tik");
		Asset = AM->LoadAsset<MOHPC::TIKI>("/models/static/bush_full.tik");
		//Asset = AM->LoadAsset<MOHPC::TIKI>("/models/human/allied_airborne_soldier.tik");
		//Asset = AM->LoadAsset<MOHPC::TIKI>("/models/player/allied_airborne.tik");
		if (Asset)
		{
			MOHPC::ModelRendererPtr ModelRenderer = MOHPC::ModelRenderer::create(AM);
			ModelRenderer->AddModel(Asset.get());
			ModelRenderer->BuildBonesTransform();
			ModelRenderer->BuildRenderData();
		}

		MOHPC::TIKIPtr Tiki = AM->LoadAsset<MOHPC::TIKI>("/models/static/toilet.tik");
		if (Tiki)
		{
			MOHPC::ModelRendererPtr ModelRenderer = MOHPC::ModelRenderer::create(AM);
			ModelRenderer->AddModel(Tiki.get());
			ModelRenderer->BuildBonesTransform();
			ModelRenderer->BuildRenderData();
		}
	}
};
static CTikiTest unitTest;
