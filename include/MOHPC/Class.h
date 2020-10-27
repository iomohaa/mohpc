#pragma once

#include "Global.h"
#include "SafePtr.h"
#include "Utilities/SharedPtr.h"
#include "Object.h"

namespace MOHPC
{
	class AssetManager;
	class FileManager;

#define CLASS_BODY			MOHPC_OBJECT_DECLARATION
#define CLASS_DEFINITION	MOHPC_OBJECT_DEFINITION

	class Class : public SafePtrClass, public Object
	{
		friend AssetManager;

		CLASS_BODY(Class);

	protected:
		MOHPC_EXPORTS Class();
		MOHPC_EXPORTS virtual ~Class();
	};
}

