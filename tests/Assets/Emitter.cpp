#include <MOHPC/Assets/Formats/TIKI.h>
#include <MOHPC/Assets/Managers/AssetManager.h>
#include <MOHPC/Utility/Managers/EmitterManager.h>
#include <MOHPC/Common/Log.h>
#include "Common/Common.h"

#include <chrono>

#define MOHPC_LOG_NAMESPACE "test_emitter"

int main(int argc, const char* argv[])
{
	const MOHPC::AssetManagerPtr AM = AssetLoad(GetGamePathFromCommandLine(argc, argv));

	MOHPC_LOG(Info, "Loading test emitter...");
	auto start = std::chrono::system_clock().now();
	MOHPC::TIKIPtr Tiki = AM->readAsset<MOHPC::TIKIReader>("/models/fx/fx_tank_explosion.tik");
	auto end = std::chrono::system_clock().now();
	MOHPC_LOG(Debug, "%lf time", std::chrono::duration<double>(end - start).count());

	MOHPC::EmitterManagerPtr EmitterManager = AM->getManager<MOHPC::EmitterManager>();

	if (Tiki)
	{
		MOHPC::EmitterResults Emitter;
		while (EmitterManager->ParseEmitters(Tiki.get(), Emitter)) {
			MOHPC_LOG(Debug, "got anim %s (%d), %d emitters", Emitter.animName.c_str(), Emitter.animNum, Emitter.GetNumEmitters());
		}
	}
}
