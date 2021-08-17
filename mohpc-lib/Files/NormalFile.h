#pragma once

#include <MOHPC/Files/File.h>

#include <fstream>

namespace MOHPC
{
	class NormalFile : public IFile
	{
	public:
		NormalFile(const fs::path& nameRef, std::ifstream&& fileRef);
		~NormalFile();

		std::istream* GetStream() override;
		uint64_t ReadBuffer(void** Out) override;

	private:
		std::ifstream fileStream;
		char* buffer;
		std::streamoff streamSize;
	};
}
