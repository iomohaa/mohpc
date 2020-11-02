#include <MOHPC/Formats/TIKI.h>
#include <MOHPC/Managers/AssetManager.h>
#include <MOHPC/Managers/EmitterManager.h>
#include <MOHPC/Log.h>
#include "UnitTest.h"

#define MOHPC_LOG_NAMESPACE "test_emitter"

class CEmitterTest : public IUnitTest
{
public:
	virtual const char* name() override
	{
		return "Emitter";
	}

	virtual void run(const MOHPC::AssetManagerPtr& AM) override
	{
		MOHPC_LOG(Log, "Loading test emitter...");
		auto start = std::chrono::system_clock().now();
		MOHPC::TIKIPtr Tiki = AM->LoadAsset<MOHPC::TIKI>("/models/fx/fx_tank_explosion.tik");
		auto end = std::chrono::system_clock().now();
		MOHPC_LOG(Verbose, "%lf time", std::chrono::duration<double>(end - start).count());

		MOHPC::EmitterManagerPtr EmitterManager = AM->GetManager<MOHPC::EmitterManager>();

		if (Tiki)
		{
			MOHPC::EmitterResults Emitter;
			while (EmitterManager->ParseEmitters(Tiki.get(), Emitter)) {
				MOHPC_LOG(Verbose, "got anim %s (%d), %d emitters", Emitter.animName.c_str(), Emitter.animNum, Emitter.GetNumEmitters());
			}
		}
	}
};
static CEmitterTest unitTest;
