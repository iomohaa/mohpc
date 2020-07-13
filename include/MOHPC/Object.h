#pragma once

#include "Global.h"
#include "Utilities/SharedPtr.h"
#include "Utilities/WeakPtr.h"
#include <utility>

namespace MOHPC
{
	class AssetManager;
}

namespace MOHPC
{
#define MOHPC_OBJECT_DECLARATION(c) \
	public: \
	template<typename...Args> static SharedPtr<c> create(Args&&...args) { return makePtr(new (allocate()) c(std::forward<Args>(args)...)); } \
	private: \
	MOHPC_EXPORTS static SharedPtr<c> makePtr(c* ThisPtr); \
	MOHPC_EXPORTS static void destroy(c* instance); \
	MOHPC_EXPORTS static void* allocate(); \

#define MOHPC_OBJECT_DEFINITION(c) \
	MOHPC::SharedPtr<c> c::makePtr(c* ThisPtr) { return MOHPC::SharedPtr<c>(ThisPtr, &c::destroy); } \
	void c::destroy(c* instance) \
	{ \
		/** Extract type from namespace */ \
		using Type = c; \
		/** Call destructor */ \
		instance->~Type(); \
		/** Free up memory */ \
		delete[] reinterpret_cast<unsigned char*>(instance); \
	} \
	void* c::allocate() { return new unsigned char[sizeof(c)]; };

	class Object
	{
		MOHPC_OBJECT_DECLARATION(Object);

	private:
		WeakPtr<AssetManager> AM;

	protected:
		MOHPC_EXPORTS Object();
		MOHPC_EXPORTS Object(const SharedPtr<AssetManager>& assetManager);
		MOHPC_EXPORTS virtual ~Object();

		Object(const Object& Object) = default;
		Object& operator=(const Object& Object) = default;
		Object(Object&& Object) noexcept = default;
		Object& operator=(Object&& Object) noexcept = default;

	public:
		/** Init the asset manager property. */
		MOHPC_EXPORTS void InitAssetManager(const SharedPtr<AssetManager>& assetManager);

		/** Init the asset manager property from another class. */
		MOHPC_EXPORTS void InitAssetManager(const Object* obj);

		/** Wrapper to AssetManager::GetManager */
		template<class T>
		SharedPtr<T> GetManager() const
		{
			SharedPtr<AssetManager> AssetManager = GetAssetManager();
			return AssetManager ? AssetManager->GetManager<T>() : nullptr;
		}

		MOHPC_EXPORTS SharedPtr<AssetManager> GetAssetManager() const;
		MOHPC_EXPORTS class FileManager* GetFileManager() const;
	};
}
