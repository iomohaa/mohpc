#include <MOHPC/Assets/AssetBase.h>
#include <MOHPC/Assets/Managers/AssetManager.h>

using namespace MOHPC;

MOHPC_OBJECT_DEFINITION(AssetObject);

AssetObject::AssetObject()
{
}

AssetObject::AssetObject(const AssetManagerPtr& AssetManager)
	: AssetObject()
{
	AM = AssetManager;
}

AssetObject::~AssetObject()
{
}

SharedPtr<MOHPC::AssetManager> AssetObject::GetAssetManager() const
{
	return AM.lock();
}

IFileManager* AssetObject::GetFileManager() const
{
	return !AM.expired() ? AM.lock()->GetFileManager().get() : nullptr;
}

FileCategoryManager* AssetObject::GetFileCategoryManager() const
{
	return !AM.expired() ? AM.lock()->GetFileCategoryManager().get() : nullptr;
}

void AssetObject::InitAssetManager(const AssetManagerPtr& assetManager)
{
	AM = assetManager;
}

void AssetObject::InitAssetManager(const AssetObject* obj)
{
	AM = obj->AM;
}
