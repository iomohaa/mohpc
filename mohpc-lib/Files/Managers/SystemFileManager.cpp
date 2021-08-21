#include <MOHPC/Files/Managers/SystemFileManager.h>
#include <MOHPC/Files/Category.h>
#include <MOHPC/Files/FileHelpers.h>
#include <MOHPC/Files/FileMap.h>
#include <MOHPC/Common/Log.h>

#include "../NormalFile.h"

using namespace MOHPC;

static constexpr char MOHPC_LOG_NAMESPACE[] = "sysfileman";

namespace MOHPC
{
	struct GamePath
	{
	public:
		GamePath(const fs::path& directoryRef, const FileCategory* categoryPtr)
			: directory(directoryRef)
			, category(categoryPtr)
		{}

		const fs::path& getDirectory() const
		{
			return directory;
		}

		const FileCategory* getCategory() const
		{
			return category;
		}

	private:
		fs::path directory;
		const FileCategory* category;
	};
}

MOHPC_OBJECT_DEFINITION(SystemFileManager);
SystemFileManager::SystemFileManager()
{
}

SystemFileManager::~SystemFileManager()
{
}

bool SystemFileManager::AddDirectory(const fs::path& directory, const FileCategory* category)
{
	const char* const categoryName = category ? category->getName() : "";

	if (!fs::exists(directory))
	{
		MOHPC_LOG(Error, "Directory '%s' for '%s' doesn't exist.", directory.generic_string().c_str(), categoryName);
		return false;
	}

	if (findGamePath(directory))
	{
		// already exists
		return false;
	}

	GamePath& G = paths.emplace_back(directory, category);

	MOHPC_LOG(Info, "Added directory '%s' for '%s'.", directory.generic_string().c_str(), categoryName);

	return true;
}

bool SystemFileManager::FileExists(const fs::path& fileName, const FileCategory* category)
{
	fs::path fileNameP = FileHelpers::removeRootDir(fileName.c_str());

	if (category == nullptr)
	{
		for (auto it = paths.begin(); it != paths.end(); ++it)
		{
			if (fileExists(*it, fileNameP)) {
				return true;
			}
		}
	}
	else
	{
		// Find file locally
		for (auto it = paths.begin(); it != paths.end(); ++it)
		{
			if (it->getCategory() == category || it->getCategory()->hasParent(category))
			{
				if (fileExists(*it, fileNameP))
				{
					// stop the iteration
					return true;
				}
			}
		}
	}

	return false;
}

IFilePtr SystemFileManager::OpenFile(const fs::path& fileName, const FileCategory* category)
{
	IFilePtr file;
	fs::path fileNameP = FileHelpers::removeRootDir(fileName.c_str());

	if (category == nullptr)
	{
		for (auto it = paths.begin(); it != paths.end(); ++it)
		{
			file = openFile(*it, std::move(fileNameP));
			if (file) {
				return file;
			}
		}
	}
	else
	{
		// Find file locally
		for (auto it = paths.begin(); it != paths.end(); ++it)
		{
			if (it->getCategory() == category || it->getCategory()->hasParent(category))
			{
				file = openFile(*it, std::move(fileNameP));
				if (file) {
					break;
				}
			}
		}
	}

	return file;
}

static bool SortFileList(const fs::path& s1, const fs::path& s2)
{
	return FileNameCompare()(s1, s2);
}

template<typename T>
static bool AreFilesEqual(const T* s1, const T* s2)
{
	return !strHelpers::icmp(s1, s2);
}

static void InsertVirtualFile(const fs::path& dir, const fs::path& path, const fs::path& requestedDir, std::set<FileEntry, FileNameCompare>& fileList)
{
	if (!path.empty())
	{
		const fs::path::string_type pathString = path.native();
		const fs::path::value_type* const virtualPath = FileHelpers::removeRootDir(
			FileNameCompare::StripBase(pathString.c_str(), dir.c_str())
		);

		const fs::path::value_type* const requestedDirP = requestedDir.c_str();
		const size_t requestedDirLength = requestedDir.native().length();

		if (!strHelpers::icmpn(virtualPath, requestedDirP, requestedDirLength))
		{
			if (fs::is_directory(virtualPath))
			{
				fs::path virtualFolderPath = fs::path(
					fs::path::string_type(virtualPath) + FileHelpers::dirSlash<fs::path::value_type>,
					fs::path::generic_format
				).generic_string<fs::path::value_type>();

				fileList.insert(std::move(virtualFolderPath));
			}
			else {
				fileList.insert(virtualPath);
			}
		}
	}
}

static void InsertVirtualFile(const fs::path& dir, const fs::path& path, const fs::path& requestedDir, const ExtensionList& extensions, std::set<FileEntry, FileNameCompare>& fileList)
{
	if (!path.empty())
	{
		const fs::path::string_type pathString = path.native();
		fs::path virtualPath = fs::path(
			FileHelpers::removeRootDir(
				FileNameCompare::StripBase(pathString.c_str(), dir.c_str())
			)
		).generic_string<fs::path::value_type>();

		const fs::path::value_type* const requestedDirP = requestedDir.c_str();
		const size_t requestedDirLength = requestedDir.native().length();

		if (!strHelpers::icmpn(virtualPath.c_str(), requestedDirP, requestedDirLength))
		{
			const fs::path::value_type* fileExtension = FileHelpers::getExtension(virtualPath.c_str());
			for (auto it = extensions.begin(); it != extensions.end(); ++it)
			{
				if (!strHelpers::icmp(it->c_str(), fileExtension))
				{
					const fs::path::value_type* fileFolder = FileHelpers::getLastPath(virtualPath.c_str());
					const size_t folderLength = fileFolder - virtualPath.c_str();
					if (folderLength)
					{
						// insert the parent folder
						fileList.insert(fs::path(virtualPath.c_str(), virtualPath.c_str() + folderLength + 1));
					}

					fileList.insert(std::move(virtualPath));

					break;
				}
			}
		}
	}
}

void SystemFileManager::ListFilteredFilesInternal(filteredFiles_t& fileList, const std::filesystem::path& directory, const ExtensionList& extensions, bool isRecursive, const FileCategory* category)
{
	const fs::path::value_type* requestedDirP = FileHelpers::removeRootDir(directory.c_str());
	const size_t requestedDirLength = directory.native().length() - (requestedDirP - directory.c_str());

	for (auto it = paths.begin(); it != paths.end(); ++it)
	{
		if (category)
		{
			if (!it->getCategory() || it->getCategory()->hasParent(category)) {
				continue;
			}
		}

		const fs::path& currentDir = it->getDirectory();
		fs::path requestedFullDir;
		if (*requestedDirP) {
			requestedFullDir = fs::path::string_type(currentDir).append(requestedDirP, 1, std::string::npos);
		}
		else {
			requestedFullDir = currentDir;
		}

		try
		{
			using namespace std::filesystem;

			if (isRecursive)
			{
				auto jt = fs::recursive_directory_iterator(requestedFullDir, directory_options::skip_permission_denied | directory_options::follow_directory_symlink);
				const auto end = fs::recursive_directory_iterator();

				if (extensions.size() > 0)
				{
					for (; jt != end; jt++) {
						InsertVirtualFile(currentDir, jt->path(), requestedDirP, extensions, fileList);
					}
				}
				else
				{
					for (; jt != end; jt++) {
						InsertVirtualFile(currentDir, jt->path(), requestedDirP, fileList);
					}
				}
			}
			else
			{
				auto jt = fs::directory_iterator(requestedFullDir, directory_options::skip_permission_denied | directory_options::follow_directory_symlink);
				const auto end = fs::directory_iterator();

				if (extensions.size() > 0)
				{
					for (; jt != end; jt++) {
						InsertVirtualFile(currentDir, jt->path(), requestedDirP, extensions, fileList);
					}
				}
				else
				{
					for (; jt != end; jt++) {
						InsertVirtualFile(currentDir, jt->path(), requestedDirP, fileList);
					}
				}
			}
		}
		catch (std::filesystem::filesystem_error&)
		{

		}
	}
}

const GamePath* SystemFileManager::findGamePath(const std::filesystem::path& path) const
{
	for (auto it = paths.begin(); it != paths.end(); ++it)
	{
		if (it->getDirectory() == path)
		{
			// found one that matches
			return &*it;
		}
	}

	return nullptr;
}

bool SystemFileManager::fileExists(const GamePath& path, const std::filesystem::path& fileName) const
{
	fs::path fullPath = path.getDirectory() / fileName;

	return fs::exists(fullPath);
}

IFilePtr SystemFileManager::openFile(const GamePath& path, std::filesystem::path&& fileName) const
{
	const fs::path fullPath = path.getDirectory() / fileName;

	std::ifstream ifs;
	ifs.open(fullPath, std::ios::in | std::ios::binary);

	if (ifs.is_open()) {
		return SharedPtr<IFile>(new NormalFile(std::move(fileName), std::move(ifs)));
	}

	return nullptr;
}

MOHPC_OBJECT_DEFINITION(SystemFileManagerProxy);
