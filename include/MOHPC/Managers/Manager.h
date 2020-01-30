#pragma once

#include "../Global.h"
#include "../Class.h"

namespace MOHPC
{
	class AssetManager;
	class FileManager;

	class Manager : public Class
	{
		friend class AssetManager;

	protected:
		MOHPC_EXPORTS Manager();
		virtual ~Manager();

		MOHPC_EXPORTS virtual void Init();
	};
}
