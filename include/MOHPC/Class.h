#pragma once

#include "Global.h"
#include "SafePtr.h"
#include "Utilities/SharedPtr.h"
#include "Object.h"

namespace MOHPC
{
	class AssetManager;
	class FileManager;

	/*
#define CLASS_BODY(c) \
	public: \
	MOHPC_EXPORTS static c* CreateInstance(); \
	MOHPC_EXPORTS static SharedPtr<c> CreatePtr(); \
	MOHPC_EXPORTS static void Delete(c* instance); \
	private:

#define CLASS_DEFINITION(c) \
	c* c::CreateInstance() { return new c(); } \
	SharedPtr<c> c::CreatePtr() { return SharedPtr<c>(new c(), &c::Delete); } \
	void c::Delete(c* instance) { delete instance; }
	*/

#define CLASS_BODY			MOHPC_OBJECT_DECLARATION
#define CLASS_DEFINITION	MOHPC_OBJECT_DEFINITION

	class Class : public SafePtrClass, public Object
	{
		friend AssetManager;

		CLASS_BODY(Class);

	protected:
		MOHPC_EXPORTS Class();
		MOHPC_EXPORTS virtual ~Class();

	public:
		/** Init the asset manager property. */
		//MOHPC_EXPORTS void InitAssetManager(AssetManager* AM);

		/** Init the asset manager property from another class. */
		//MOHPC_EXPORTS void InitAssetManager(const Class* Object);
	};
}

