#pragma once

#include <MOHPC/Files/File.h>

#include <istream>

namespace MOHPC
{
	class ArchiveFile : public IFile
	{
	public:
		ArchiveFile(const fs::path& nameRef, void* file, const void* pos, uint64_t uncompressedSize);
		~ArchiveFile();

		std::istream* GetStream() override;
		uint64_t ReadBuffer(void** Out) override;

	private:
		std::basic_streambuf<char>* streamBuf;
		std::istream archiveStream;
		char* buffer;
		std::streamoff archiveSize;
	};
}
