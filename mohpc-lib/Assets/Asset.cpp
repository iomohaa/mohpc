#include <Shared.h>
#include <MOHPC/Assets/Asset.h>
#include <MOHPC/Assets/Managers/AssetManager.h>
#include <MOHPC/Files/Managers/FileManager.h>
#include <MOHPC/Common/Log.h>
#include <MOHPC/Version.h>
#include "Utility/Misc/SHA1.h"

using namespace MOHPC;

#define MOHPC_LOG_NAMESPACE "assetmanager"

MOHPC_OBJECT_DEFINITION(AssetManager);

/*
template<>
intptr_t mfuse::Hash<std::type_index>::operator()(const std::type_index& key) const
{
	return key.hash_code();
}
*/

AssetManager::AssetManager()
{
	FM = NULL;

	MOHPC_LOG(Debug, "MOHPC %s version %s build %d", VERSION_ARCHITECTURE, VERSION_SHORT_STRING, VERSION_BUILD);
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
	mfuse::con::set_enum en(m_managers);
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
	m_managers.insert_or_assign(ti, manager);
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

	const auto it = m_managers.find(ti);
	if (it != m_managers.end()) {
		return it->second;
	}

	return nullptr;
}

SharedPtr<Asset> AssetManager::CacheFindAsset(const char *Filename)
{
	auto it = m_assetCache.find(Filename);
	if (it != m_assetCache.end())
	{
		WeakPtr<Asset>& asset = it->second;

		if(!asset.expired())
		{
			// Lock and return the shared pointer
			return asset.lock();
		}
		else
		{
			// Not keeping a null asset
			m_assetCache.erase(it);
		}
	}
	return nullptr;
}

bool AssetManager::CacheLoadAsset(const char *Filename, const SharedPtr<Asset>& A)
{
	A->InitAssetManager(shared_from_this());
	A->Init(Filename);
	A->Load();

	A->HashFinalize();

	//m_assetCache[Filename] = A;
	//m_assetCache.addKeyValue(Filename) = A;
	m_assetCache.insert_or_assign(Filename, A);

	MOHPC_LOG(Info, "Asset '%s' loaded", Filename);
	return true;
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

void Asset::HashUpdate(const uint8_t* Data, uint64_t Length)
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

AssetError::AssetNotFound::AssetNotFound(const str& inFileName)
	: fileName(inFileName)
{
}

const char* AssetError::AssetNotFound::getFileName() const
{
	return fileName.c_str();
}

const char* AssetError::AssetNotFound::what() const noexcept
{
	return "The specified asset was not found";
}

AssetError::BadHeader4::BadHeader4(const uint8_t inFoundHeader[4], const uint8_t inExpectedHeader[4])
	: foundHeader{ inFoundHeader[0], inFoundHeader[1], inFoundHeader[2], inFoundHeader[3] }
	, expectedHeader{ inExpectedHeader[0], inExpectedHeader[1], inExpectedHeader[2], inExpectedHeader[3] }
{

}

const uint8_t* AssetError::BadHeader4::getHeader() const
{
	return foundHeader;
}

const uint8_t* AssetError::BadHeader4::getExpectedHeader() const
{
	return expectedHeader;
}

const char* AssetError::BadHeader4::what() const noexcept
{
	return "Bad asset header";
}