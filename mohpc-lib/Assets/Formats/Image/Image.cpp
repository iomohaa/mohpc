#include <Shared.h>
#include <MOHPC/Assets/Formats/Image.h>
#include <MOHPC/Assets/Managers/AssetManager.h>
#include <MOHPC/Files/Managers/FileManager.h>
#include <string>

using namespace MOHPC;

MOHPC_OBJECT_DEFINITION(Image);
Image::Image()
{
	data = nullptr;
	dataSize = 0;
	width = height = 0;
	pixelFormat = PixelFormat::Unknown;
}

Image::~Image()
{
	if (data)
	{
		delete[] data;
	}
}

void Image::Load()
{
	struct ExtensionWrapper
	{
		const char *Extension;

		typedef void (Image::*_ExtensionFunction)(const char *, void *, uint64_t);
		_ExtensionFunction ExtensionFunction;
	};

	static ExtensionWrapper ExtensionWrappers[] =
	{
		{ "jpg", &Image::LoadJPEG },
		{ "jpeg", &Image::LoadJPEG },
		{ "tga", &Image::LoadTGA },
		{ "dds", &Image::LoadDDS }
	};

	const char* ext = GetFilename().GetExtension();

	ExtensionWrapper::_ExtensionFunction ExtensionFunction = nullptr;

	const size_t numExtensions = sizeof(ExtensionWrappers) / sizeof(ExtensionWrappers[0]);
	for (intptr_t i = 0; i < numExtensions; i++)
	{
		if (!str::icmp(ext, ExtensionWrappers[i].Extension))
		{
			ExtensionFunction = ExtensionWrappers[i].ExtensionFunction;
			break;
		}
	}

	if (!ExtensionFunction) {
		throw ImageError::BadExtension(ext);
	}

	const char *Fname = GetFilename().c_str();

	FilePtr file = GetFileManager()->OpenFile(Fname);
	if (!file) {
		throw AssetError::AssetNotFound(GetFilename());
	}

	void *buf;
	uint64_t len = file->ReadBuffer(&buf);

	(this->*ExtensionFunction)(Fname, buf, len);

	HashUpdate((uint8_t*)buf, (size_t)len);
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

ImageError::BadExtension::BadExtension(const str& inExtension)
	: extension(inExtension)
{
}

const char* ImageError::BadExtension::getExtension() const
{
	return extension.c_str();
}

const char* ImageError::BadExtension::what() const noexcept
{
	return "Bad or unsupported image extension";
}

ImageError::HeaderTooShort::HeaderTooShort(uint64_t inLen)
	: len(inLen)
{
}

uint64_t ImageError::HeaderTooShort::getLength() const
{
	return len;
}

const char* ImageError::HeaderTooShort::what() const noexcept
{
	return "Image header too short";
}

ImageError::FileTruncated::FileTruncated(uint64_t inExpectedSize, uint64_t inFileSize)
	: expectedSize(inExpectedSize)
	, fileSize(inFileSize)
{
}

uint64_t ImageError::FileTruncated::getExpectedSize() const
{
	return expectedSize;
}

uint64_t ImageError::FileTruncated::getFileSize() const
{
	return fileSize;
}

const char* ImageError::FileTruncated::what() const noexcept
{
	return "The image file is truncated";
}
