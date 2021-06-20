#pragma once

#include "../AssetsGlobal.h"
#include "../AssetBase.h"

namespace MOHPC
{
	class AssetManager;
	class FileManager;

	class Manager : public AssetObject
	{
		friend class AssetManager;

	protected:
		MOHPC_ASSETS_EXPORTS Manager();
		virtual ~Manager();

		MOHPC_ASSETS_EXPORTS virtual void Init();
	};
}
