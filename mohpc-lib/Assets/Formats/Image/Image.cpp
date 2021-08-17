#include <Shared.h>
#include <MOHPC/Assets/Formats/Image.h>
#include <MOHPC/Assets/Managers/AssetManager.h>
#include <MOHPC/Files/Managers/FileManager.h>
#include <MOHPC/Files/FileHelpers.h>
#include <string>

using namespace MOHPC;

MOHPC_OBJECT_DEFINITION(Image);

Image::Image(const fs::path& fileNameRef, uint8_t* dataBuf, uint32_t size, uint32_t widthVal, uint32_t heightVal, PixelFormat format)
	: Asset2(fileNameRef)
	, data(dataBuf)
	, dataSize(size)
	, width(widthVal)
	, height(heightVal)
	, pixelFormat(format)
{
}

Image::~Image()
{
	if (data)
	{
		delete[] data;
	}
}

MOHPC_OBJECT_DEFINITION(ImageReader);

Asset2Ptr ImageReader::read(const IFilePtr& file)
{
	struct ExtensionWrapper
	{
		const fs::path Extension;

		typedef ImagePtr (ImageReader::*_ExtensionFunction)(const fs::path&, void *, uint64_t);
		_ExtensionFunction ExtensionFunction;
	};

	static ExtensionWrapper ExtensionWrappers[] =
	{
		{ "jpg", &ImageReader::LoadJPEG },
		{ "jpeg", &ImageReader::LoadJPEG },
		{ "tga", &ImageReader::LoadTGA },
		{ "dds", &ImageReader::LoadDDS }
	};

	const fchar_t* ext = FileHelpers::getExtension(file->getName().c_str());

	ExtensionWrapper::_ExtensionFunction ExtensionFunction = nullptr;

	const size_t numExtensions = sizeof(ExtensionWrappers) / sizeof(ExtensionWrappers[0]);
	for (intptr_t i = 0; i < numExtensions; i++)
	{
		if (!strHelpers::icmp(ext, ExtensionWrappers[i].Extension.c_str()))
		{
			ExtensionFunction = ExtensionWrappers[i].ExtensionFunction;
			break;
		}
	}

	if (!ExtensionFunction) {
		throw ImageError::BadExtension(ext);
	}

	void *buf;
	uint64_t len = file->ReadBuffer(&buf);

	return (this->*ExtensionFunction)(file->getName(), buf, len);
}

ImageReader::ImageReader()
{
}

ImageReader::~ImageReader()
{
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

ImageError::BadExtension::BadExtension(const fs::path& inExtension)
	: extension(inExtension)
{
}

const fs::path& ImageError::BadExtension::getExtension() const
{
	return extension;
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
