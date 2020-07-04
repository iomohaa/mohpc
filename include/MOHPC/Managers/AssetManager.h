#pragma once

#include "../Global.h"
#include "../Asset.h"
#include "../Utilities/SharedPtr.h"
#include "../Utilities/WeakPtr.h"
#include "../Script/con_set.h"
#include "Manager.h"
#include <typeinfo>
#include <typeindex>
#include "../Class.h"

namespace MOHPC
{
	class AssetManager
	{
		friend class Class;

	public:
		MOHPC_EXPORTS AssetManager();
		MOHPC_EXPORTS ~AssetManager();

	public:
		MOHPC_EXPORTS FileManager * GetFileManager() const;
		MOHPC_EXPORTS void SetFileManager(FileManager* FileManager);

		/** Returns a manager class. */
		template<class T>
		T* GetManager()
		{
			T* manager = nullptr;
			if (std::is_base_of<Manager, T>::value)
			{
				const std::type_index ti = typeid(T);
				manager = dynamic_cast<T*>(GetManager(ti));
				if (manager == nullptr && !bPendingDestroy)
				{
					manager = T::CreateInstance();
					manager->AM = this;
					AddManager(ti, static_cast<Manager*>(manager));
					((Manager*)manager)->Init();
				}
			}
			return static_cast<T*>(manager);
		}

		// The following code doesn't compile with the cancerous linux g++ compiler
		/*
		template<class T, class ...Args>
		T* CreateAsset()
		{
		T* A = new T(Args ...);
		InitAsset(A);
		return A;
		}
		*/

#if 0
		/** Create a new object from the asset manager. */
		template<class T>
		T* NewObject()
		{
			T* C = nullptr;
			if (std::is_base_of<Class, T>::value)
			{
				C = (T*)AllocObject(sizeof(T));
				if (C)
				{
					new (C) T;
				}
			}
			return C;
		}

		/** Create a new object from the asset manager. */
		template<class T, typename ...Args>
		T* NewObject(Args... args)
		{
			T* C = nullptr;
			if (std::is_base_of<Class, T>::value)
			{
				C = (T*)AllocObject(sizeof(T));
				if (C)
				{
					new (C) T(args...);
				}
			}
			return C;
		}

		template<class T>
		T* NewObjects(size_t count)
		{
			assert(count);

			if (std::is_base_of<Class, T>::value)
			{
				T* C = (T*)AllocObjects(sizeof(T), count);
				if (C)
				{
					for (uintptr_t i = 0; i < count; i++)
					{
						new (C + i) T();
					}
				}
				return C;
			}
			else
			{
				return nullptr;
			}
		}

		/*
		template<class T>
		void DeleteObject(T* A)
		{
			static_assert(!std::is_base_of<Asset, T>::value, "You must call UnloadAsset in order to unload assets.");
		}
		*/
#endif

		/** Loads an asset from disk/paks. */
		template<class T>
		SharedPtr<T> LoadAsset(const char *Filename)
		{
			SharedPtr<T> A = staticPointerCast<T>(CacheFindAsset(Filename));
			if (!A)
			{
				A = SharedPtr<T>(T::CreateInstance(), &T::Delete);
				if (!CacheLoadAsset(Filename, A)) {
					return nullptr;
				}
			}
			return A;
		}

#if 0
		/** Unloads an asset. */
		void UnloadAsset(Asset* A)
		{
			CacheUnloadAsset(A);
		}
#endif

	private:
		MOHPC_EXPORTS void AddManager(const std::type_index& ti, Manager* manager);
		MOHPC_EXPORTS Manager* GetManager(const std::type_index& ti) const;
		MOHPC_EXPORTS SharedPtr<Asset> CacheFindAsset(const char *Filename);
		MOHPC_EXPORTS bool CacheLoadAsset(const char *Filename, SharedPtr<Asset> A);
		void CacheUnloadAsset(Asset* A);
		void DeleteUnreferencedAssets();
		void *AllocObject(size_t ObjectSize);
		void *AllocObjects(size_t ObjectSize, size_t Count);
		void InitializeObject(Class* Object);
		void DestructObject(Class* Object);
		void DestructObjects(Class* Object);
		void DeleteObject(Class* Object);
		void DeleteObjects(Class* Object);

		bool bPendingDestroy;
		mutable FileManager* FM;
		con_set<std::type_index, Manager*> m_managers;
		con_set<str, WeakPtr<Asset>> m_assetCache;
		//std::unordered_set<void*> m_allocatedObjects;
		//std::unordered_set<void*> m_allocatedArrayObjects;
	};

	template<class T>
	T* Class::GetManager() const
	{
		AssetManager* AM = GetAssetManager();
		return AM ? AM->GetManager<T>() : nullptr;
	}
}
