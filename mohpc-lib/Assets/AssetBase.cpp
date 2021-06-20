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

class FileManager* AssetObject::GetFileManager() const
{
	return !AM.expired() ? AM.lock()->GetFileManager() : nullptr;
}

void AssetObject::InitAssetManager(const AssetManagerPtr& assetManager)
{
	AM = assetManager;
}

void AssetObject::InitAssetManager(const AssetObject* obj)
{
	AM = obj->AM;
}

