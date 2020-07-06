#include <MOHPC/Formats/TIKI.h>
#include <MOHPC/Managers/AssetManager.h>
#include <MOHPC/Managers/EmitterManager.h>
#include "UnitTest.h"

class CEmitterTest : public IUnitTest, public TAutoInst<CEmitterTest>
{
public:
	virtual const char* name() override
	{
		return "Emitter";
	}

	virtual void run(const MOHPC::AssetManagerPtr& AM) override
	{
		printf("Loading test emitter...\n");
		auto start = std::chrono::system_clock().now();
		MOHPC::TIKIPtr Tiki = AM->LoadAsset<MOHPC::TIKI>("/models/fx/fx_tank_explosion.tik");
		auto end = std::chrono::system_clock().now();
		printf("%lf time\n", std::chrono::duration<double>(end - start).count());

		MOHPC::EmitterManager* EmitterManager = AM->GetManager<MOHPC::EmitterManager>();

		if (Tiki)
		{
			MOHPC::EmitterResults Emitter;
			while (EmitterManager->ParseEmitters(Tiki.get(), Emitter));
		}
	}
};
