#pragma once

#include "Manager.h"
#include "../Asset.h"
#include "../../Common/Object.h"
#include "../../Files/FileDefs.h"
#include "../../Files/FileMap.h"
#include "../../Utility/SharedPtr.h"
#include "../../Utility/WeakPtr.h"

#include <utility>
#include <unordered_map>
#include <typeinfo>
#include <typeindex>
#include <exception>

namespace MOHPC
{
	class IFileManager;
	class FileCategoryManager;

	class AssetManager : public std::enable_shared_from_this<AssetManager>
	{
		friend class Class;

		MOHPC_ASSET_OBJECT_DECLARATION(AssetManager);

	private:
		MOHPC_ASSETS_EXPORTS AssetManager(const SharedPtr<IFileManager>& FMptr, const SharedPtr<FileCategoryManager>& catManPtr);
		~AssetManager();

	public:
		/** Get the virtual file manager associated with the asset manager. */
		MOHPC_ASSETS_EXPORTS const SharedPtr<IFileManager>& GetFileManager() const;

		/** Get the virtual file manager associated with the asset manager. */
		MOHPC_ASSETS_EXPORTS const SharedPtr<FileCategoryManager>& GetFileCategoryManager() const;

		/**
		 * Return a class used to share data globally across assets.
		 *
		 * @tparam Type of the manager.
		 */
		template<class T>
		SharedPtr<T> getManager()
		{
			static_assert(std::is_base_of<Manager, T>::value, "T must be a subclass of Manager");

			const std::type_index ti = typeid(T);
			SharedPtr<T> manager = staticPointerCast<T>(getManager(ti));
			if (manager == nullptr)
			{
				manager = T::create();
				addManager(ti, manager);
			}
			return manager;
		}

		/**
		 * Load an asset from disk or pak files.
		 *
		 * @tparam T asset reader to use.
		 * @param fileName virtual path to file.
		 * @param args... arguments to pass to the reader constructor.
		 * @return Shared pointer to asset if found, NULL otherwise.
		 * @note If the asset is already loaded, it is returned from cache.
		 */
		template<typename T, typename...Args>
		SharedPtr<typename T::AssetType> readAsset(const fs::path& fileName, Args&&...args)
		{
			using AssetType = typename T::AssetType;
			SharedPtr<AssetType> A = staticPointerCast<AssetType>(cacheFindAsset(fileName));
			if (!A)
			{
				SharedPtr<T> reader = T::create(std::forward<Args>(args)...);
				A = staticPointerCast<AssetType>(readAsset(fileName, reader));
			}

			return A;
		}

		/**
		 * Load an asset from disk or pak files.
		 *
		 * @tparam T asset reader to use.
		 * @param file pointer to a file interface.
		 * @param args... arguments to pass to the reader constructor.
		 * @return Shared pointer to asset if found, NULL otherwise.
		 * @note If the asset is already loaded, it is returned from cache.
		 */
		template<class T, typename...Args>
		SharedPtr<typename T::AssetType> readAsset(const IFilePtr& file, Args&&...args)
		{
			using AssetType = typename T::AssetType;
			SharedPtr<T> reader = T::create(std::forward<Args>(args)...);
			return staticPointerCast<AssetType>(readAsset(file, reader));
		}

	private:
		MOHPC_ASSETS_EXPORTS void addManager(const std::type_index& ti, const SharedPtr<Manager>& manager);
		MOHPC_ASSETS_EXPORTS SharedPtr<Manager> getManager(const std::type_index& ti) const;
		MOHPC_ASSETS_EXPORTS SharedPtr<Asset> cacheFindAsset(const fs::path& Filename);
		MOHPC_ASSETS_EXPORTS SharedPtr<Asset> readAsset(const fs::path& fileName, const SharedPtr<AssetReader>& A);
		MOHPC_ASSETS_EXPORTS SharedPtr<Asset> readAsset(const IFilePtr& file, const SharedPtr<AssetReader>& A);

	private:
		SharedPtr<IFileManager> FM;
		SharedPtr<FileCategoryManager> catMan;

		/**
		 * Shared pointer, because each manager is a storage, part of the asset manager
		 * and must be destroyed during or after AssetManager destruction.
		 */
		std::unordered_map<std::type_index, SharedPtr<Manager>> m_managers;

		/**
		 * Weak pointer, because it's just a cache of loaded assets.
		 * They can be destroyed anytime.
		 */
		std::unordered_map<fs::path, WeakPtr<Asset>, FileNameHash, FileNameMapCompare> m_assetCache;
	};

	using AssetManagerPtr = SharedPtr<AssetManager>;

	template<class T>
	SharedPtr<T> AssetObject::getManager() const
	{
		const SharedPtr<AssetManager> manPtr = GetAssetManager();
		if (manPtr)
		{
			// return the owning asset manager
			return manPtr->getManager<T>();
		}

		return nullptr;
	}

	namespace AssetError
	{
		class Base : public std::exception {};

		class AssetNotFound : public Base
		{
		public:
			AssetNotFound(const fs::path& inFileName);

			MOHPC_ASSETS_EXPORTS const fs::path& getFileName() const;

		public:
			const char* what() const noexcept override;

		private:
			fs::path fileName;
		};

		/**
		 * The asset file has wrong header.
		 */
		class BadHeader4 : public Base
		{
		public:
			BadHeader4(const uint8_t foundHeader[4], const uint8_t expectedHeader[4]);

			MOHPC_ASSETS_EXPORTS const uint8_t* getHeader() const;
			MOHPC_ASSETS_EXPORTS const uint8_t* getExpectedHeader() const;

		public:
			const char* what() const noexcept override;

		private:
			uint8_t foundHeader[4];
			uint8_t expectedHeader[4];
		};
	}
}
