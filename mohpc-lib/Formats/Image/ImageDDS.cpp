#include <Shared.h>
#include <MOHPC/Formats/Image.h>
#include <MOHPC/Misc/Endian.h>
#include <cstring>
#include "ImagePrivate.h"

using namespace MOHPC;

typedef unsigned int   ui32_t;

typedef struct ddsHeader_s
{
	ui32_t headerSize;
	ui32_t flags;
	ui32_t height;
	ui32_t width;
	ui32_t pitchOrFirstMipSize;
	ui32_t volumeDepth;
	ui32_t numMips;
	ui32_t reserved1[11];
	ui32_t always_0x00000020;
	ui32_t pixelFormatFlags;
	ui32_t fourCC;
	ui32_t rgbBitCount;
	ui32_t rBitMask;
	ui32_t gBitMask;
	ui32_t bBitMask;
	ui32_t aBitMask;
	ui32_t caps;
	ui32_t caps2;
	ui32_t caps3;
	ui32_t caps4;
	ui32_t reserved2;
}
ddsHeader_t;

// flags:
#define _DDSFLAGS_REQUIRED     0x001007
#define _DDSFLAGS_PITCH        0x8
#define _DDSFLAGS_MIPMAPCOUNT  0x20000
#define _DDSFLAGS_FIRSTMIPSIZE 0x80000
#define _DDSFLAGS_VOLUMEDEPTH  0x800000

// pixelFormatFlags:
#define DDSPF_ALPHAPIXELS 0x1
#define DDSPF_ALPHA       0x2
#define DDSPF_FOURCC      0x4
#define DDSPF_RGB         0x40
#define DDSPF_YUV         0x200
#define DDSPF_LUMINANCE   0x20000

// caps:
#define DDSCAPS_COMPLEX  0x8
#define DDSCAPS_MIPMAP   0x400000
#define DDSCAPS_REQUIRED 0x1000

// caps2:
#define DDSCAPS2_CUBEMAP 0xFE00
#define DDSCAPS2_VOLUME  0x200000

typedef struct ddsHeaderDxt10_s
{
	ui32_t dxgiFormat;
	ui32_t dimensions;
	ui32_t miscFlags;
	ui32_t arraySize;
	ui32_t miscFlags2;
}
ddsHeaderDxt10_t;

// dxgiFormat
// from http://msdn.microsoft.com/en-us/library/windows/desktop/bb173059%28v=vs.85%29.aspx
typedef enum DXGI_FORMAT {
	DXGI_FORMAT_UNKNOWN = 0,
	DXGI_FORMAT_R32G32B32A32_TYPELESS = 1,
	DXGI_FORMAT_R32G32B32A32_FLOAT = 2,
	DXGI_FORMAT_R32G32B32A32_UINT = 3,
	DXGI_FORMAT_R32G32B32A32_SINT = 4,
	DXGI_FORMAT_R32G32B32_TYPELESS = 5,
	DXGI_FORMAT_R32G32B32_FLOAT = 6,
	DXGI_FORMAT_R32G32B32_UINT = 7,
	DXGI_FORMAT_R32G32B32_SINT = 8,
	DXGI_FORMAT_R16G16B16A16_TYPELESS = 9,
	DXGI_FORMAT_R16G16B16A16_FLOAT = 10,
	DXGI_FORMAT_R16G16B16A16_UNORM = 11,
	DXGI_FORMAT_R16G16B16A16_UINT = 12,
	DXGI_FORMAT_R16G16B16A16_SNORM = 13,
	DXGI_FORMAT_R16G16B16A16_SINT = 14,
	DXGI_FORMAT_R32G32_TYPELESS = 15,
	DXGI_FORMAT_R32G32_FLOAT = 16,
	DXGI_FORMAT_R32G32_UINT = 17,
	DXGI_FORMAT_R32G32_SINT = 18,
	DXGI_FORMAT_R32G8X24_TYPELESS = 19,
	DXGI_FORMAT_D32_FLOAT_S8X24_UINT = 20,
	DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS = 21,
	DXGI_FORMAT_X32_TYPELESS_G8X24_UINT = 22,
	DXGI_FORMAT_R10G10B10A2_TYPELESS = 23,
	DXGI_FORMAT_R10G10B10A2_UNORM = 24,
	DXGI_FORMAT_R10G10B10A2_UINT = 25,
	DXGI_FORMAT_R11G11B10_FLOAT = 26,
	DXGI_FORMAT_R8G8B8A8_TYPELESS = 27,
	DXGI_FORMAT_R8G8B8A8_UNORM = 28,
	DXGI_FORMAT_R8G8B8A8_UNORM_SRGB = 29,
	DXGI_FORMAT_R8G8B8A8_UINT = 30,
	DXGI_FORMAT_R8G8B8A8_SNORM = 31,
	DXGI_FORMAT_R8G8B8A8_SINT = 32,
	DXGI_FORMAT_R16G16_TYPELESS = 33,
	DXGI_FORMAT_R16G16_FLOAT = 34,
	DXGI_FORMAT_R16G16_UNORM = 35,
	DXGI_FORMAT_R16G16_UINT = 36,
	DXGI_FORMAT_R16G16_SNORM = 37,
	DXGI_FORMAT_R16G16_SINT = 38,
	DXGI_FORMAT_R32_TYPELESS = 39,
	DXGI_FORMAT_D32_FLOAT = 40,
	DXGI_FORMAT_R32_FLOAT = 41,
	DXGI_FORMAT_R32_UINT = 42,
	DXGI_FORMAT_R32_SINT = 43,
	DXGI_FORMAT_R24G8_TYPELESS = 44,
	DXGI_FORMAT_D24_UNORM_S8_UINT = 45,
	DXGI_FORMAT_R24_UNORM_X8_TYPELESS = 46,
	DXGI_FORMAT_X24_TYPELESS_G8_UINT = 47,
	DXGI_FORMAT_R8G8_TYPELESS = 48,
	DXGI_FORMAT_R8G8_UNORM = 49,
	DXGI_FORMAT_R8G8_UINT = 50,
	DXGI_FORMAT_R8G8_SNORM = 51,
	DXGI_FORMAT_R8G8_SINT = 52,
	DXGI_FORMAT_R16_TYPELESS = 53,
	DXGI_FORMAT_R16_FLOAT = 54,
	DXGI_FORMAT_D16_UNORM = 55,
	DXGI_FORMAT_R16_UNORM = 56,
	DXGI_FORMAT_R16_UINT = 57,
	DXGI_FORMAT_R16_SNORM = 58,
	DXGI_FORMAT_R16_SINT = 59,
	DXGI_FORMAT_R8_TYPELESS = 60,
	DXGI_FORMAT_R8_UNORM = 61,
	DXGI_FORMAT_R8_UINT = 62,
	DXGI_FORMAT_R8_SNORM = 63,
	DXGI_FORMAT_R8_SINT = 64,
	DXGI_FORMAT_A8_UNORM = 65,
	DXGI_FORMAT_R1_UNORM = 66,
	DXGI_FORMAT_R9G9B9E5_SHAREDEXP = 67,
	DXGI_FORMAT_R8G8_B8G8_UNORM = 68,
	DXGI_FORMAT_G8R8_G8B8_UNORM = 69,
	DXGI_FORMAT_BC1_TYPELESS = 70,
	DXGI_FORMAT_BC1_UNORM = 71,
	DXGI_FORMAT_BC1_UNORM_SRGB = 72,
	DXGI_FORMAT_BC2_TYPELESS = 73,
	DXGI_FORMAT_BC2_UNORM = 74,
	DXGI_FORMAT_BC2_UNORM_SRGB = 75,
	DXGI_FORMAT_BC3_TYPELESS = 76,
	DXGI_FORMAT_BC3_UNORM = 77,
	DXGI_FORMAT_BC3_UNORM_SRGB = 78,
	DXGI_FORMAT_BC4_TYPELESS = 79,
	DXGI_FORMAT_BC4_UNORM = 80,
	DXGI_FORMAT_BC4_SNORM = 81,
	DXGI_FORMAT_BC5_TYPELESS = 82,
	DXGI_FORMAT_BC5_UNORM = 83,
	DXGI_FORMAT_BC5_SNORM = 84,
	DXGI_FORMAT_B5G6R5_UNORM = 85,
	DXGI_FORMAT_B5G5R5A1_UNORM = 86,
	DXGI_FORMAT_B8G8R8A8_UNORM = 87,
	DXGI_FORMAT_B8G8R8X8_UNORM = 88,
	DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM = 89,
	DXGI_FORMAT_B8G8R8A8_TYPELESS = 90,
	DXGI_FORMAT_B8G8R8A8_UNORM_SRGB = 91,
	DXGI_FORMAT_B8G8R8X8_TYPELESS = 92,
	DXGI_FORMAT_B8G8R8X8_UNORM_SRGB = 93,
	DXGI_FORMAT_BC6H_TYPELESS = 94,
	DXGI_FORMAT_BC6H_UF16 = 95,
	DXGI_FORMAT_BC6H_SF16 = 96,
	DXGI_FORMAT_BC7_TYPELESS = 97,
	DXGI_FORMAT_BC7_UNORM = 98,
	DXGI_FORMAT_BC7_UNORM_SRGB = 99,
	DXGI_FORMAT_AYUV = 100,
	DXGI_FORMAT_Y410 = 101,
	DXGI_FORMAT_Y416 = 102,
	DXGI_FORMAT_NV12 = 103,
	DXGI_FORMAT_P010 = 104,
	DXGI_FORMAT_P016 = 105,
	DXGI_FORMAT_420_OPAQUE = 106,
	DXGI_FORMAT_YUY2 = 107,
	DXGI_FORMAT_Y210 = 108,
	DXGI_FORMAT_Y216 = 109,
	DXGI_FORMAT_NV11 = 110,
	DXGI_FORMAT_AI44 = 111,
	DXGI_FORMAT_IA44 = 112,
	DXGI_FORMAT_P8 = 113,
	DXGI_FORMAT_A8P8 = 114,
	DXGI_FORMAT_B4G4R4A4_UNORM = 115,
	DXGI_FORMAT_FORCE_UINT = 0xffffffffUL
} DXGI_FORMAT;

#define EncodeFourCC(x) ((((ui32_t)((x)[0]))      ) | \
                         (((ui32_t)((x)[1])) << 8 ) | \
                         (((ui32_t)((x)[2])) << 16) | \
                         (((ui32_t)((x)[3])) << 24) )

#define GL_COMPRESSED_SRGB_EXT            0x8C48
#define GL_COMPRESSED_SRGB_ALPHA_EXT      0x8C49
#define GL_COMPRESSED_SLUMINANCE_EXT      0x8C4A
#define GL_COMPRESSED_SLUMINANCE_ALPHA_EXT 0x8C4B
#define GL_COMPRESSED_SRGB_S3TC_DXT1_EXT  0x8C4C
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT 0x8C4D
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT 0x8C4E
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT 0x8C4F

#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT   0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT  0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT  0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT  0x83F3

#define GL_COMPRESSED_RED_RGTC1           0x8DBB
#define GL_COMPRESSED_SIGNED_RED_RGTC1    0x8DBC
#define GL_COMPRESSED_RG_RGTC2            0x8DBD
#define GL_COMPRESSED_SIGNED_RG_RGTC2     0x8DBE

#define GL_COMPRESSED_RGBA_BPTC_UNORM_ARB 0x8E8C
#define GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_ARB 0x8E8D
#define GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB 0x8E8E
#define GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB 0x8E8F

#define GL_SRGB8_ALPHA8_EXT               0x8C43

#define GL_RGBA8                          0x8058

static constexpr char DDS_HEADER[] = "DDS ";
static constexpr char DDS_HEADER_DX10[] = "DX10";

void Image::LoadDDS(const char *name, void *buf, uint64_t len)
{
	//
	// reject files that are too small to hold even a header
	//
	if (len < 4 + sizeof(ddsHeader_t))
	{
		// it is too small to be a DDS file
		throw ImageError::HeaderTooShort(len);
	}

	//
	// reject files that don't start with "DDS "
	//
	const uint32_t header = Endian.LittleInteger(*(uint32_t*)buf);
	if (memcmp(&header, DDS_HEADER, sizeof(header)))
	{
		// not a DDS file
		throw ImageError::DDS::BadHeader((const uint8_t*)&header);
	}

	//
	// parse header and dx10 header if available
	//
	const uint8_t* ddsData;
	const ddsHeaderDxt10_t* ddsHeaderDxt10 = nullptr;
	const ddsHeader_t* ddsHeader = (const ddsHeader_t *)((const uint8_t*)buf + 4);
	const uint32_t pixelFormatFlags = Endian.LittleInteger(ddsHeader->pixelFormatFlags);
	const uint32_t fourCC = Endian.LittleInteger(ddsHeader->fourCC);
	if ((pixelFormatFlags & DDSPF_FOURCC) && !memcmp(&fourCC, DDS_HEADER_DX10, sizeof(fourCC)))
	{
		static constexpr size_t minHeaderSize = 4 + sizeof(*ddsHeader) + sizeof(*ddsHeaderDxt10);

		if (len < minHeaderSize)
		{
			// indicates a DX10 header it is too small to contain
			throw ImageError::HeaderTooShort(len);
		}

		ddsHeaderDxt10 = (const ddsHeaderDxt10_t *)((const uint8_t*)buf + 4 + sizeof(ddsHeader_t));
		ddsData = (const uint8_t*)buf + 4 + sizeof(*ddsHeader) + sizeof(*ddsHeaderDxt10);
		len -= 4 + sizeof(*ddsHeader) + sizeof(*ddsHeaderDxt10);
	}
	else
	{
		ddsData = (const uint8_t*)buf + 4 + sizeof(*ddsHeader);
		len -= 4 + sizeof(*ddsHeader);
	}

	width = Endian.LittleInteger(ddsHeader->width);
	height = Endian.LittleInteger(ddsHeader->height);

	uint32_t picFormat = 0;

	//
	// Convert DXGI format/FourCC into OpenGL format
	//
	if (ddsHeaderDxt10)
	{
		const uint32_t dxgiFormat = Endian.LittleInteger(ddsHeaderDxt10->dxgiFormat);

		switch (dxgiFormat)
		{
		case DXGI_FORMAT_BC1_TYPELESS:
		case DXGI_FORMAT_BC1_UNORM:
			// FIXME: check for GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
			picFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
			break;

		case DXGI_FORMAT_BC1_UNORM_SRGB:
			// FIXME: check for GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT
			picFormat = GL_COMPRESSED_SRGB_S3TC_DXT1_EXT;
			break;

		case DXGI_FORMAT_BC2_TYPELESS:
		case DXGI_FORMAT_BC2_UNORM:
			picFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
			break;

		case DXGI_FORMAT_BC2_UNORM_SRGB:
			picFormat = GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT;
			break;

		case DXGI_FORMAT_BC3_TYPELESS:
		case DXGI_FORMAT_BC3_UNORM:
			picFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
			break;

		case DXGI_FORMAT_BC3_UNORM_SRGB:
			picFormat = GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT;
			break;

		case DXGI_FORMAT_BC4_TYPELESS:
		case DXGI_FORMAT_BC4_UNORM:
			picFormat = GL_COMPRESSED_RED_RGTC1;
			break;

		case DXGI_FORMAT_BC4_SNORM:
			picFormat = GL_COMPRESSED_SIGNED_RED_RGTC1;
			break;

		case DXGI_FORMAT_BC5_TYPELESS:
		case DXGI_FORMAT_BC5_UNORM:
			picFormat = GL_COMPRESSED_RG_RGTC2;
			break;

		case DXGI_FORMAT_BC5_SNORM:
			picFormat = GL_COMPRESSED_SIGNED_RG_RGTC2;
			break;

		case DXGI_FORMAT_BC6H_TYPELESS:
		case DXGI_FORMAT_BC6H_UF16:
			picFormat = GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB;
			break;

		case DXGI_FORMAT_BC6H_SF16:
			picFormat = GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB;
			break;

		case DXGI_FORMAT_BC7_TYPELESS:
		case DXGI_FORMAT_BC7_UNORM:
			picFormat = GL_COMPRESSED_RGBA_BPTC_UNORM_ARB;
			break;

		case DXGI_FORMAT_BC7_UNORM_SRGB:
			picFormat = GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_ARB;
			break;

		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
			picFormat = GL_SRGB8_ALPHA8_EXT;
			break;

		case DXGI_FORMAT_R8G8B8A8_UNORM:
		case DXGI_FORMAT_R8G8B8A8_SNORM:
			picFormat = GL_RGBA8;
			break;

		default:
			throw ImageError::DDS::UnsupportedDXGIFormat(dxgiFormat);
		}
	}
	else
	{
		const uint32_t rgbBitCount = Endian.LittleInteger(ddsHeader->rgbBitCount);
		const uint32_t rBitMask = Endian.LittleInteger(ddsHeader->rBitMask);
		const uint32_t gBitMask = Endian.LittleInteger(ddsHeader->gBitMask);
		const uint32_t bBitMask = Endian.LittleInteger(ddsHeader->bBitMask);
		const uint32_t aBitMask = Endian.LittleInteger(ddsHeader->aBitMask);

		if (pixelFormatFlags & DDSPF_FOURCC)
		{
			if (fourCC == EncodeFourCC("DXT1"))
				picFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
			else if (fourCC == EncodeFourCC("DXT2"))
				picFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
			else if (fourCC == EncodeFourCC("DXT3"))
				picFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
			else if (fourCC == EncodeFourCC("DXT4"))
				picFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
			else if (fourCC == EncodeFourCC("DXT5"))
				picFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
			else if (fourCC == EncodeFourCC("ATI1"))
				picFormat = GL_COMPRESSED_RED_RGTC1;
			else if (fourCC == EncodeFourCC("BC4U"))
				picFormat = GL_COMPRESSED_RED_RGTC1;
			else if (fourCC == EncodeFourCC("BC4S"))
				picFormat = GL_COMPRESSED_SIGNED_RED_RGTC1;
			else if (fourCC == EncodeFourCC("ATI2"))
				picFormat = GL_COMPRESSED_RG_RGTC2;
			else if (fourCC == EncodeFourCC("BC5U"))
				picFormat = GL_COMPRESSED_RG_RGTC2;
			else if (fourCC == EncodeFourCC("BC5S"))
				picFormat = GL_COMPRESSED_SIGNED_RG_RGTC2;
			else
				throw ImageException("DDS File %s has unsupported FourCC.", name);
		}
		else if (pixelFormatFlags == (DDSPF_RGB | DDSPF_ALPHAPIXELS)
			&& rgbBitCount == 32
			&& rBitMask == 0x000000ff
			&& gBitMask == 0x0000ff00
			&& bBitMask == 0x00ff0000
			&& aBitMask == 0xff000000)
		{
			picFormat = GL_RGBA8;
		}
		else {
			throw ImageException("DDS File %s has unsupported RGBA format.", name);
		}
	}

	data = new uint8_t[(size_t)len];
	memcpy(data, ddsData, (size_t)len);
	pixelFormat = PixelFormat::RGBA;
}

ImageError::DDS::BadHeader::BadHeader(const uint8_t inFoundHeader[4])
	: foundHeader{ inFoundHeader[0], inFoundHeader[1], inFoundHeader[2], inFoundHeader[3] }
{
}

const uint8_t* ImageError::DDS::BadHeader::getHeader() const
{
	return foundHeader;
}

const char* ImageError::DDS::BadHeader::what() const
{
	return "Bad DDS image header";
}

ImageError::DDS::UnsupportedDXGIFormat::UnsupportedDXGIFormat(uint32_t inFormat)
	: format(inFormat)
{
}

uint32_t ImageError::DDS::UnsupportedDXGIFormat::getFormat() const
{
	return format;
}

const char* ImageError::DDS::UnsupportedDXGIFormat::what() const
{
	return "Unsupported DXGI format";
}

ImageError::DDS::UnsupportedFourCCFormat::UnsupportedFourCCFormat(uint32_t inFormat)
	: format(inFormat)
{
}

uint32_t ImageError::DDS::UnsupportedFourCCFormat::getFormat() const
{
	return format;
}

const char* ImageError::DDS::UnsupportedFourCCFormat::what() const
{
	return "Unsupported four-character code";
}

ImageError::DDS::UnsupportedRGBAFormat::UnsupportedRGBAFormat(uint32_t inFormat)
	: format(inFormat)
{
}

uint32_t ImageError::DDS::UnsupportedRGBAFormat::getFormat() const
{
	return format;
}

const char* ImageError::DDS::UnsupportedRGBAFormat::what() const
{
	return "Unsupported RGBA format";
}
