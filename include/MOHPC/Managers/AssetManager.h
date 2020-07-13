#pragma once

#include "../Global.h"
#include "../Asset.h"
#include "../Object.h"
#include "../Utilities/SharedPtr.h"
#include "../Utilities/WeakPtr.h"
#include "../Script/con_set.h"
#include "Manager.h"
#include <typeinfo>
#include <typeindex>
#include "../Class.h"

namespace MOHPC
{
	class AssetManager : public std::enable_shared_from_this<AssetManager>
	{
		friend class Class;

		MOHPC_OBJECT_DECLARATION(AssetManager);

	private:
		MOHPC_EXPORTS AssetManager();
		~AssetManager();

	public:
		/** Get the virtual file manager associated with the asset manager. */
		MOHPC_EXPORTS FileManager* GetFileManager() const;

		/** Return a manager class check the *Managers* folder. */
		template<class T>
		SharedPtr<T> GetManager()
		{
			static_assert(std::is_base_of<Manager, T>::value);

			const std::type_index ti = typeid(T);
			SharedPtr<T> manager = staticPointerCast<T>(GetManager(ti));
			if (manager == nullptr)
			{
				manager = T::create();
				AddManager(ti, manager);
			}
			return manager;
		}

		/** 
		 * Load an asset from disk or pak files.
		 *
		 * @param	Filename	Virtual path to file.
		 * @return	Shared pointer to asset if found, NULL otherwise.
		 * @note	If the asset is already loaded, it is returned from cache.
		 */
		template<class T>
		SharedPtr<T> LoadAsset(const char *Filename)
		{
			SharedPtr<T> A = staticPointerCast<T>(CacheFindAsset(Filename));
			if (!A)
			{
				A = T::create();
				if (!CacheLoadAsset(Filename, A)) {
					return nullptr;
				}
			}
			return A;
		}

	private:
		MOHPC_EXPORTS void AddManager(const std::type_index& ti, const SharedPtr<Manager>& manager);
		MOHPC_EXPORTS SharedPtr<Manager> GetManager(const std::type_index& ti) const;
		MOHPC_EXPORTS SharedPtr<Asset> CacheFindAsset(const char *Filename);
		MOHPC_EXPORTS bool CacheLoadAsset(const char *Filename, const SharedPtr<Asset>& A);

	private:
		mutable FileManager* FM;

		/**
		 * Shared pointer, because each manager is a storage, part of the asset manager
		 * and must be destroyed during or after AssetManager destruction.
		 */
		con_set<std::type_index, SharedPtr<Manager>> m_managers;

		/**
		 * Weak pointer, because it's just a cache of loaded assets.
		 * They can be destroyed anytime.
		 */
		con_set<str, WeakPtr<Asset>> m_assetCache;
	};
	using AssetManagerPtr = SharedPtr<AssetManager>;
}
