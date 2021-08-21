#include <MOHPC/Files/Category.h>
#include <MOHPC/Common/str.h>

using namespace MOHPC;

MOHPC_OBJECT_DEFINITION(FileCategoryManager);

FileCategoryManager::FileCategoryManager()
{

}

FileCategoryManager::~FileCategoryManager()
{
	for (auto it = list.begin(); it != list.end(); ++it)
	{
		FileCategory* cat = *it;
		delete cat;
	}
}

FileCategory* FileCategoryManager::createCategory(const char* name)
{
	FileCategory* const cat = new FileCategory(name);
	categories.insert(cat);
	list.push_back(cat);
	return cat;
}

FileCategory* FileCategoryManager::createCategory(FileCategory* parent, const char* name)
{
	FileCategory* const cat = new FileCategory(parent, name);
	categories.insert(cat);
	list.push_back(cat);
	return cat;
}

FileCategory* FileCategoryManager::findCategory(const char* name)
{
	for (auto it = categories.begin(); it != categories.end(); ++it)
	{
		FileCategory* cat = *it;
		if (!strHelpers::cmp(cat->name.c_str(), name)) {
			return cat;
		}
	}

	return nullptr;
}

void FileCategoryManager::removeCategory(FileCategory* category)
{
	categories.erase(category);
	list.remove(category);

	// no longer valid
	if (category->parent) {
		category->parent->children.erase(category);
	}

	// clear parent from children
	for (auto it = category->children.begin(); it != category->children.end(); ++it)
	{
		FileCategory* const child = *it;

		if (child->parent == category) {
			child->parent = nullptr;
		}
	}
}

void FileCategoryManager::GetCategoryList(std::vector<const FileCategory*>& outList) const
{
	outList.resize(list.size());
	std::copy(list.begin(), list.end(), outList.begin());
}

FileCategory::FileCategory(const char* nameValue) noexcept
	: name(nameValue)
	, parent(nullptr)
{
}

FileCategory::FileCategory(FileCategory* parentPtr, const char* nameValue) noexcept
	: name(nameValue)
	, parent(parentPtr)
{
}

const char* FileCategory::getName() const noexcept
{
	return name.c_str();
}

const FileCategory* FileCategory::getParent() const noexcept
{
	return parent;
}

bool FileCategory::hasParent(const FileCategory* category) const noexcept
{
	for (const FileCategory* cat = this; cat != nullptr; cat = cat->parent)
	{
		if (cat == category) {
			return true;
		}
	}

	return false;
}

const FileCategory* FileCategory::getLast() const noexcept
{
	if (children.size())
	{
		const MOHPC::FileCategory* child = *children.rbegin();
		return child->getLast();
	}
	else {
		return this;
	}
}

FileCategory::~FileCategory()
{
}
