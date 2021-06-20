#pragma once

#include "AssetsGlobal.h"
#include "../Common/Object.h"

namespace MOHPC
{
#define MOHPC_ASSET_OBJECT_DECLARATION(c) MOHPC_OBJECT_DECLARATION_BASE(c, MOHPC_ASSETS_EXPORTS)

	class AssetObject
	{
		MOHPC_ASSET_OBJECT_DECLARATION(AssetObject);

	private:
		WeakPtr<AssetManager> AM;

	protected:
		MOHPC_ASSETS_EXPORTS AssetObject();
		MOHPC_ASSETS_EXPORTS AssetObject(const SharedPtr<AssetManager>& assetManager);
		MOHPC_ASSETS_EXPORTS virtual ~AssetObject();

		AssetObject(const AssetObject& Object) = default;
		AssetObject& operator=(const AssetObject& Object) = default;
		AssetObject(AssetObject&& Object) noexcept = default;
		AssetObject& operator=(AssetObject&& Object) noexcept = default;

	public:
		/** Init the asset manager property. */
		MOHPC_ASSETS_EXPORTS void InitAssetManager(const SharedPtr<AssetManager>& assetManager);

		/** Init the asset manager property from another class. */
		MOHPC_ASSETS_EXPORTS void InitAssetManager(const AssetObject* obj);

		/** Wrapper to AssetManager::GetManager */
		template<class T>
		SharedPtr<T> GetManager() const;

		MOHPC_ASSETS_EXPORTS SharedPtr<AssetManager> GetAssetManager() const;
		MOHPC_ASSETS_EXPORTS class FileManager* GetFileManager() const;
	};
}
