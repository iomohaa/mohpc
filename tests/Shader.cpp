#include <MOHPC/Formats/Image.h>
#include <MOHPC/Managers/AssetManager.h>
#include <MOHPC/Managers/ShaderManager.h>

void TestShader(MOHPC::AssetManager& AM)
{
	printf("Loading shaders...\n");

	MOHPC::ShaderManager* SM = AM.GetManager<MOHPC::ShaderManager>();

	auto Shader = SM->GetShader("textures/common/caulk");
	if (Shader && Shader->GetNumStages())
	{
		const MOHPC::Image* img = Shader->GetStage(0)->bundle[0].image[0]->GetImage();
	}
}
