#include <Shared.h>
#include <MOHPC/Formats/Image.h>
#include <MOHPC/Managers/FileManager.h>
#include <filesystem>
#include <string>
#include <string.h>

using namespace std::string_literals;

using namespace MOHPC;
namespace fs = std::filesystem;

CLASS_DEFINITION(Image);
Image::Image()
{
	data = nullptr;
	dataSize = 0;
	width = height = 0;
	pixelFormat = PixelFormat::IF_Unknown;
}

Image::~Image()
{
	if (data)
	{
		delete[] data;
	}
}

bool Image::Load()
{
	struct ExtensionWrapper
	{
		const char *Extension;

		typedef void (Image::*_ExtensionFunction)(const char *, void *, std::streamsize);
		_ExtensionFunction ExtensionFunction;
	};

	static ExtensionWrapper ExtensionWrappers[] =
	{
		{ ".jpg", &Image::LoadJPEG },
		{ ".jpeg", &Image::LoadJPEG },
		{ ".tga", &Image::LoadTGA },
		{ ".dds", &Image::LoadDDS }
	};

	std::string ext = fs::path(GetFilename().c_str()).extension().generic_string();

	ExtensionWrapper::_ExtensionFunction ExtensionFunction = nullptr;

	const size_t numExtensions = sizeof(ExtensionWrappers) / sizeof(ExtensionWrappers[0]);
	for (intptr_t i = 0; i < numExtensions; i++)
	{
		if (!stricmp(ext.c_str(), ExtensionWrappers[i].Extension))
		{
			ExtensionFunction = ExtensionWrappers[i].ExtensionFunction;
			break;
		}
	}

	if (!ExtensionFunction)
	{
		return false;
	}

	const char *Fname = GetFilename().c_str();

	FilePtr file = GetFileManager()->OpenFile(Fname);
	if (!file)
	{
		return false;
	}

	void *buf;
	std::streamsize len = file->ReadBuffer(&buf);

	try
	{
		(this->*ExtensionFunction)(Fname, buf, len);
	}
	catch (std::exception&)
	{
		return false;
	}

	HashUpdate((uint8_t*)buf, (size_t)len);

	return true;
}

uint8_t *Image::GetData() const
{
	return data;
}

uint32_t Image::GetDataSize() const
{
	return dataSize;
}

PixelFormat Image::GetPixelFormat() const
{
	return pixelFormat;
}

uint32_t Image::GetWidth() const
{
	return width;
}

uint32_t Image::GetHeight() const
{
	return height;
}
