#include <MOHPC/Formats/Image.h>
#include <MOHPC/Managers/AssetManager.h>
#include <MOHPC/Managers/ShaderManager.h>
#include <MOHPC/Log.h>
#include "UnitTest.h"

#define MOHPC_LOG_NAMESPACE "test_shader"

class CShaderTest : public IUnitTest
{
public:
	virtual unsigned int priority() override
	{
		return 1;
	}

	virtual const char* name() override
	{
		return "Shader";
	}

	virtual void run(const MOHPC::AssetManagerPtr& AM) override
	{
		MOHPC_LOG(Info, "Loading shaders...");

		MOHPC::ShaderManagerPtr SM = AM->GetManager<MOHPC::ShaderManager>();

		auto Shader = SM->GetShader("textures/common/caulk");
		if (Shader && Shader->GetNumStages())
		{
			const MOHPC::Image* img = Shader->GetStage(0)->bundle[0].image[0]->GetImage();
		}
	}
};
static CShaderTest unitTest;
