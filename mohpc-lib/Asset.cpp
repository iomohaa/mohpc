#include <Shared.h>
#include <MOHPC/Asset.h>
#include <MOHPC/Managers/AssetManager.h>
#include <MOHPC/Managers/FileManager.h>
#include <MOHPC/Log.h>
#include <MOHPC/Version.h>
#include "Misc/SHA1.h"

using namespace MOHPC;

#define MOHPC_LOG_NAMESPACE "assetmanager"

MOHPC_OBJECT_DEFINITION(AssetManager);

template<>
intptr_t MOHPC::HashCode<std::type_index>(const std::type_index& key)
{
	return key.hash_code();
}

AssetManager::AssetManager()
{
	FM = NULL;

	MOHPC_LOG(Verbose, "MOHPC %s version %s build %d", VERSION_ARCHITECTURE, VERSION_SHORT_STRING, VERSION_BUILD);
}

AssetManager::~AssetManager()
{
	/*
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
	*/

	/*
	con_set_enum en(m_managers);
	for (auto it = en.NextElement(); it; it = en.NextElement())
	{
		Manager* A = it->value;
		if (A)
		{
			it->value = nullptr;
			delete A;
		}
	}
	*/

	if (FM)
	{
		// Delete the file manager
		delete FM;
	}
}

FileManager* AssetManager::GetFileManager() const
{
	if (!FM) {
		FM = new FileManager;
	}
	return FM;
}

void AssetManager::AddManager(const std::type_index& ti, const SharedPtr<Manager>& manager)
{
	//m_managers[ti] = manager;
	//manager->AM = this;
	manager->InitAssetManager(shared_from_this());
	m_managers.addKeyValue(ti) = manager;
	manager->Init();
}

SharedPtr<Manager> AssetManager::GetManager(const std::type_index& ti) const
{
	/*
	auto it = m_managers.find(ti);
	if (it != m_managers.end())
	{
		return it->second;
	}
	else
	{
		return nullptr;
	}
	*/
	const SharedPtr<Manager>* manager = m_managers.findKeyValue(ti);
	if (manager) {
		return *manager;
	}
	return nullptr;
}

SharedPtr<Asset> AssetManager::CacheFindAsset(const char *Filename)
{
	WeakPtr<Asset>* asset = m_assetCache.findKeyValue(Filename);
	if (asset)
	{
		if(!asset->expired())
		{
			// Lock and return the shared pointer
			return asset->lock();
		}
		else
		{
			// Not keeping a null asset
			m_assetCache.remove(Filename);
		}
	}
	return nullptr;
}

bool AssetManager::CacheLoadAsset(const char *Filename, const SharedPtr<Asset>& A)
{
	A->InitAssetManager(shared_from_this());
	A->Init(Filename);
	if (!A->Load())
	{
		// Can't continue
		return false;
	}

	A->HashFinalize();

	//m_assetCache[Filename] = A;
	m_assetCache.addKeyValue(Filename) = A;
	return true;
}

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

const str& Asset::GetFilename() const
{
	return Filename;
}

void Asset::HashGetHash(uint8_t* Destination) const
{
	/*
	if (Hash)
	{
		CSHA1* SHA1 = (CSHA1*)Hash;
		SHA1->GetHash(Destination);
	}
	*/
}

void Asset::HashUpdate(const uint8_t* Data, std::streamsize Length)
{
	/*
	if (!Hash)
	{
		Hash = (class Hasher*)new CSHA1;
	}

	CSHA1* SHA1 = (CSHA1*)Hash;
	SHA1->Update(Data, (UINT_32)Length);
	*/
}

void Asset::HashCopy(const Asset* A)
{
	/*
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
	*/
}

void Asset::HashFinalize()
{
	if (Hash)
	{
		CSHA1* SHA1 = (CSHA1*)Hash;
		SHA1->Final();
	}
}
