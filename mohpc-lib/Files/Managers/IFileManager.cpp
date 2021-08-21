#include <MOHPC/Files/Managers/IFileManager.h>
#include <MOHPC/Files/FileMap.h>

using namespace MOHPC;

IFileManager::IFileManager()
{
}

IFileManager::~IFileManager()
{
}

FileEntryList IFileManager::ListFilteredFiles(const std::filesystem::path& directory, const ExtensionList& extensions, bool isRecursive, const FileCategory* category)
{
	filteredFiles_t fileList;
	ListFilteredFilesInternal(fileList, directory, extensions, isRecursive, category);

	const size_t numFiles = fileList.size();
	std::vector<FileEntry> out;
	out.resize(numFiles);
	std::move(fileList.begin(), fileList.end(), out.begin());

	return out;
}

FileEntryList IFileManager::ListFilteredFiles(const std::filesystem::path& directory, const std::filesystem::path& extension, bool isRecursive, const FileCategory* category)
{
	ExtensionList list(1, extension);

	return ListFilteredFiles(directory, list, isRecursive, category);
}

FileEntryList IFileManager::ListAllFilteredFiles(const std::filesystem::path& directory, bool isRecursive, const FileCategory* category)
{
	return ListFilteredFiles(directory, ExtensionList(), isRecursive, category);
}
