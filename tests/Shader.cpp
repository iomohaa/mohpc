#include <MOHPC/Assets/Formats/Image.h>
#include <MOHPC/Assets/Managers/AssetManager.h>
#include <MOHPC/Assets/Managers/ShaderManager.h>
#include <MOHPC/Common/Log.h>
#include "Common/Common.h"

#define MOHPC_LOG_NAMESPACE "test_shader"

int main(int argc, const char* argv[])
{
	const MOHPC::AssetManagerPtr AM = AssetLoad();

	MOHPC_LOG(Info, "Loading shaders...");

	MOHPC::ShaderManagerPtr SM = AM->GetManager<MOHPC::ShaderManager>();

	auto Shader = SM->GetShader("textures/common/caulk");
	if (Shader && Shader->GetNumStages())
	{
		const MOHPC::Image* img = Shader->GetStage(0)->bundle[0].image[0]->GetImage();
	}
}
