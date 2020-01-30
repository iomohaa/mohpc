#pragma once

#include "Global.h"
#include "SafePtr.h"

namespace MOHPC
{
	class AssetManager;
	class FileManager;

#define CLASS_BODY(c) \
	public: \
	MOHPC_EXPORTS static c* CreateInstance(); \
	private:

#define CLASS_DEFINITION(c) \
	c* c::CreateInstance() { return new c(); }

	class Class : public SafePtrClass
	{
		friend AssetManager;

		CLASS_BODY(Class);

	private:
		AssetManager * AM;

	protected:
		MOHPC_EXPORTS Class();
		MOHPC_EXPORTS virtual ~Class();
		
		Class(const Class& Object);
		//Class& operator=(const Class& Object) = delete;

		// Non-moveable
		//Class(Class&& Object) = delete;
		//Class& operator=(Class&& Object) = delete;

	public:
		/** Init the asset manager property. */
		MOHPC_EXPORTS void InitAssetManager(AssetManager* AM);

		/** Init the asset manager property from another class. */
		MOHPC_EXPORTS void InitAssetManager(const Class* Object);

		/** Wrapper to AssetManager::GetManager */
		template<class T>
		T* GetManager() const;

		MOHPC_EXPORTS AssetManager* GetAssetManager() const;
		MOHPC_EXPORTS FileManager* GetFileManager() const;
	};
}

