#include <MOHPC/Assets/Formats/Image.h>
#include <MOHPC/Assets/Managers/AssetManager.h>
#include <MOHPC/Assets/Managers/ShaderManager.h>
#include <MOHPC/Common/Log.h>
#include "Common/Common.h"

#include <cassert>

#define MOHPC_LOG_NAMESPACE "test_shader"

using namespace MOHPC;

int main(int argc, const char* argv[])
{
	InitCommon(argc, argv);

	const AssetManagerPtr AM = AssetLoad(GetGamePathFromCommandLine().c_str());

	MOHPC_LOG(Info, "Loading shaders...");

	ShaderManagerPtr SM = AM->getManager<ShaderManager>();

	auto Shader = SM->GetShader("textures/common/caulk");
	assert(Shader);
	assert(!Shader->GetNumStages());

	Shader = SM->GetShader("textures/common/black");
	assert(Shader);
	assert(Shader->GetNumStages());
	ImageCache* cache = Shader->GetStage(0)->bundle[0].image[0];
	assert(cache);
	cache->CacheImage();
	const MOHPC::Image* img = cache->GetImage();
	assert(img);
}
