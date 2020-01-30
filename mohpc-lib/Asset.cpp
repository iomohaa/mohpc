#include <Shared.h>
#include <MOHPC/Asset.h>
#include <MOHPC/Managers/AssetManager.h>
#include <MOHPC/Managers/FileManager.h>
#include "Misc/SHA1.h"

using namespace MOHPC;

AssetManager::AssetManager()
{
	FM = NULL;
	bPendingDestroy = false;
}

AssetManager::~AssetManager()
{
	bPendingDestroy = true;

#if 0
	// Delete remaining objects
	for (void* Memory : m_allocatedObjects)
	{
		if (Memory)
		{
			DestructObject((Class*)Memory);
		}
	}
	for (void* Memory : m_allocatedArrayObjects)
	{
		if (Memory)
		{
			DestructObjects((Class*)Memory);
		}
	}

	for (void* Memory : m_allocatedObjects)
	{
		if (Memory)
		{
			free(Memory);
		}
	}
	for (void* Memory : m_allocatedArrayObjects)
	{
		if (Memory)
		{
			free(Memory);
		}
	}
#endif

	// Delete managers last
	for (auto& it : m_managers)
	{
		Manager* A = it.second;
		if (A)
		{
			it.second = nullptr;
			delete A;
		}
	}

	// Delete the file manager
	if (FM)
	{
		delete FM;
	}
}

FileManager* AssetManager::GetFileManager() const
{
	if (!FM)
	{
		FM = new FileManager;
	}
	return FM;
}

void AssetManager::SetFileManager(FileManager* FileManager)
{
	FM = FileManager;
}

void AssetManager::AddManager(const std::type_index& ti, Manager* manager)
{
	m_managers[ti] = manager;
}

Manager* AssetManager::GetManager(const std::type_index& ti) const
{
	auto it = m_managers.find(ti);
	if (it != m_managers.end())
	{
		return it->second;
	}
	else
	{
		return nullptr;
	}
}

std::shared_ptr<Asset> AssetManager::CacheFindAsset(const char *Filename)
{
	auto it = m_assetCache.find(Filename);
	if (it == m_assetCache.end())
	{
		return NULL;
	}
	return it->second.lock();
}

bool AssetManager::CacheLoadAsset(const char *Filename, std::shared_ptr<Asset> A)
{
	A->AM = this;
	A->Init(Filename);
	if (!A->Load())
	{
		return false;
	}

	A->HashFinalize();

	m_assetCache[Filename] = A;
	return true;
}

void AssetManager::CacheUnloadAsset(Asset* A)
{
	/*
	for (auto it = m_assetCache.begin(); it != m_assetCache.end(); it++)
	{
		Asset* EnumA = it->second;
		if (EnumA == A)
		{
			m_assetCache.erase(it);
			DeleteObject((Class*)EnumA);
			break;
		}
	}

	//DeleteUnreferencedAssets();
	*/
}

void AssetManager::DeleteUnreferencedAssets()
{
	/*
	for (auto it = m_assetCache.begin(); it != m_assetCache.end();)
	{
		AssetPtr& EnumA = it->second;
		if (!EnumA.Pointer() || EnumA.GetRefCount() <= 1)
		{
			if (EnumA.Pointer())
			{
				DeleteObject((Class*)EnumA.Pointer());
			}
			it = m_assetCache.erase(it);
		}
		else
		{
			it++;
		}
	}
	*/
}

#if 0
void *AssetManager::AllocObject(size_t ObjectSize)
{
	Class *Object = nullptr;
	void *Memory = (Class*)malloc(ObjectSize);
	if (Memory)
	{
		Object = (Class*)Memory;
		InitializeObject(Object);
		m_allocatedObjects.insert(Memory);
	}
	return Object;
}

void *AssetManager::AllocObjects(size_t ObjectSize, size_t Count)
{
	Class *Object = nullptr;
	void *Memory = (Class*)malloc(ObjectSize * Count + sizeof(size_t) * 2);
	if (Memory)
	{
		*((size_t*)Memory) = ObjectSize;
		*((size_t*)Memory + 1) = Count;

		Object = (Class*)((size_t*)Memory + 2);

		for (uintptr_t i = 0; i < Count; i++)
		{
			Class* C = (Class*)((char*)Object + ObjectSize * i);
			InitializeObject(C);
		}

		m_allocatedArrayObjects.insert(Memory);
	}
	return Object;
}

void AssetManager::InitializeObject(Class* Object)
{
	Object->AM = this;
}

void AssetManager::DeleteObject(Class* Object)
{
	m_allocatedObjects.erase(Object);
	//DestructObject(Object);
	free(Object);
}

void AssetManager::DeleteObjects(Class* Object)
{
	void *Memory = ((size_t*)Object - 2);
	m_allocatedArrayObjects.erase(Memory);
	DestructObjects(Object);
	free(Memory);
}

void AssetManager::DestructObject(Class *Object)
{
	Object->~Class();
}

void AssetManager::DestructObjects(Class *Object)
{
	void *Memory = ((size_t*)Object - 2);

	const size_t ObjectSize = *((size_t*)Memory);
	const size_t Count = *((size_t*)Memory + 1);

	for (uintptr_t i = 0; i < Count; i++)
	{
		Class* C = (Class*)((char*)Object + ObjectSize * i);
		C->~Class();
	}
}

/*
void AssetManager::DeleteObject(Asset* A)
{
	UnloadAsset(A);
}
*/
#endif

Manager::Manager()
{
}

Manager::~Manager()
{
}

void Manager::Init()
{
}

CLASS_DEFINITION(Class);
Class::Class()
{
	// AM must be already set by the asset manager
}

Class::Class(const Class& Object)
{
	AM = Object.AM;
}

Class::~Class()
{
}

#if 0
void Class::operator delete(void* ptr)
{
	Asset* A = (Asset*)ptr;
	A->GetAssetManager()->DeleteObject((Class*)ptr);
}

void Class::operator delete[](void* ptr)
{
	Asset* A = (Asset*)ptr;
	A->GetAssetManager()->DeleteObjects((Class*)ptr);
}
#endif

/*
void* Class::operator new(size_t size)
{
	return ::operator new(size);
}

void Class::operator delete(void* ptr)
{
	::operator delete(ptr);
}
*/

void Class::InitAssetManager(AssetManager* AM)
{
	this->AM = AM;
}

void Class::InitAssetManager(const Class* Object)
{
	AM = Object->AM;
}

AssetManager* Class::GetAssetManager() const
{
	return AM;
}

FileManager* Class::GetFileManager() const
{
	if (AM != NULL)
	{
		return AM->GetFileManager();
	}
	else
	{
		return NULL;
	}
}

Asset::Asset()
{
	Hash = nullptr;
}

Asset::~Asset()
{
	if (Hash)
	{
		delete (CSHA1*)Hash;
	}
}

void Asset::Init(const char *F)
{
	Filename = F;
}

bool Asset::Load()
{
	return false;
}

const std::string& Asset::GetFilename() const
{
	return Filename;
}

void Asset::HashGetHash(uint8_t* Destination) const
{
	if (Hash)
	{
		CSHA1* SHA1 = (CSHA1*)Hash;
		SHA1->GetHash(Destination);
	}
}

void Asset::HashUpdate(const uint8_t* Data, size_t Length)
{
	if (!Hash)
	{
		Hash = (class Hasher*)new CSHA1;
	}

	CSHA1* SHA1 = (CSHA1*)Hash;
	SHA1->Update(Data, (UINT_32)Length);
}

void Asset::HashCopy(const Asset* A)
{
	if (A->Hash)
	{
		if (!Hash)
		{
			Hash = (class Hasher*)new CSHA1;
		}

		CSHA1* SHA1 = (CSHA1*)Hash;
		*SHA1 = *(CSHA1*)A->Hash;
	}
	else if (Hash)
	{
		delete (CSHA1*)Hash;
		Hash = nullptr;
	}
}

void Asset::HashFinalize()
{
	if (Hash)
	{
		CSHA1* SHA1 = (CSHA1*)Hash;
		SHA1->Final();
	}
}
