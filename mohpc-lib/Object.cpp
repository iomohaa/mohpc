#include <MOHPC/Object.h>
#include <MOHPC/Managers/AssetManager.h>

using namespace MOHPC;

MOHPC_OBJECT_DEFINITION(Object);

Object::Object()
{
}

Object::Object(const AssetManagerPtr& AssetManager)
	: Object()
{
	AM = AssetManager;
}

Object::~Object()
{
}

SharedPtr<MOHPC::AssetManager> Object::GetAssetManager() const
{
	return AM.lock();
}

class FileManager* Object::GetFileManager() const
{
	return !AM.expired() ? AM.lock()->GetFileManager() : nullptr;
}

void Object::InitAssetManager(const AssetManagerPtr& assetManager)
{
	AM = assetManager;
}

void Object::InitAssetManager(const Object* obj)
{
	AM = obj->AM;
}

