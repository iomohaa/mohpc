#pragma once

#include "FilesGlobal.h"
#include "FilesObject.h"
#include "../Utility/SharedPtr.h"

#include <string>
#include <set>
#include <list>
#include <vector>

namespace MOHPC
{
	class FileCategory
	{
	public:
		FileCategory(const char* name) noexcept;
		FileCategory(FileCategory* parent, const char* name) noexcept;
		~FileCategory();

		MOHPC_FILES_EXPORTS const char* getName() const noexcept;
		MOHPC_FILES_EXPORTS const FileCategory* getParent() const noexcept;
		MOHPC_FILES_EXPORTS const FileCategory* getLast() const noexcept;
		MOHPC_FILES_EXPORTS bool hasParent(const FileCategory* category) const noexcept;

	public:
		std::string name;
		FileCategory* parent;
		std::set<FileCategory*> children;
	};

	class FileCategoryManager
	{
		MOHPC_FILES_OBJECT_DECLARATION(FileCategoryManager);

	public:
		MOHPC_FILES_EXPORTS FileCategoryManager();
		MOHPC_FILES_EXPORTS ~FileCategoryManager();

		MOHPC_FILES_EXPORTS FileCategory* createCategory(const char* name);
		MOHPC_FILES_EXPORTS FileCategory* createCategory(FileCategory* parent, const char* name);
		MOHPC_FILES_EXPORTS FileCategory* findCategory(const char* name);
		MOHPC_FILES_EXPORTS void removeCategory(FileCategory* category);

		/**
		 * Return the list of category names
		 *
		 * @param OutList - The list of category names.
		 */
		MOHPC_FILES_EXPORTS void GetCategoryList(std::vector<const FileCategory*>& outList) const;

	private:
		std::set<FileCategory*> categories;
		std::list<FileCategory*> list;
	};
	using FileCategoryManagerPtr = SharedPtr<FileCategoryManager>;

	template<typename T>
	bool forEachCategory(const FileCategory* category, T func)
	{
		const FileCategory* current = category;
		while (current)
		{
			if (!func(current)) {
				return false;
			}
			current = current->getParent();
		}

		return true;
	}
}
