#include <Shared.h>
#include <MOHPC/Formats/Image.h>
#include <MOHPC/Misc/Endian.h>
#include "ImagePrivate.h"
#include <string.h>

using namespace MOHPC;

typedef struct _TargaHeader {
	unsigned char 	id_length, colormap_type, image_type;
	unsigned short	colormap_index, colormap_length;
	unsigned char	colormap_size;
	unsigned short	x_origin, y_origin, width, height;
	unsigned char	pixel_size, attributes;
} TargaHeader;

void Image::LoadTGA(const char *name, void *buf, std::streamsize len)
{
	int32_t columns, rows, numPixels;
	uint8_t *pixbuf;
	int32_t row, column;
	TargaHeader	targa_header;
	uint8_t *targa_rgba;

	if (len < 18) {
		throw ImageException("LoadTGA: header too short (%s)", name);
	}

	const uint8_t* buf_p = (uint8_t*)buf;
	const uint8_t* end = (uint8_t*)buf + len;

	targa_header.id_length = buf_p[0];
	targa_header.colormap_type = buf_p[1];
	targa_header.image_type = buf_p[2];

	memcpy(&targa_header.colormap_index, &buf_p[3], 2);
	memcpy(&targa_header.colormap_length, &buf_p[5], 2);
	targa_header.colormap_size = buf_p[7];
	memcpy(&targa_header.x_origin, &buf_p[8], 2);
	memcpy(&targa_header.y_origin, &buf_p[10], 2);
	memcpy(&targa_header.width, &buf_p[12], 2);
	memcpy(&targa_header.height, &buf_p[14], 2);
	targa_header.pixel_size = buf_p[16];
	targa_header.attributes = buf_p[17];

	targa_header.colormap_index = Endian.LittleShort(targa_header.colormap_index);
	targa_header.colormap_length = Endian.LittleShort(targa_header.colormap_length);
	targa_header.x_origin = Endian.LittleShort(targa_header.x_origin);
	targa_header.y_origin = Endian.LittleShort(targa_header.y_origin);
	targa_header.width = Endian.LittleShort(targa_header.width);
	targa_header.height = Endian.LittleShort(targa_header.height);

	buf_p += 18;

	if (targa_header.image_type != 2
		&& targa_header.image_type != 10
		&& targa_header.image_type != 3)
	{
		throw ImageException("LoadTGA: Only type 2 (RGB), 3 (gray), and 10 (RGB) TGA images supported");
	}

	if (targa_header.colormap_type != 0)
	{
		throw ImageException("LoadTGA: colormaps not supported");
	}

	if ((targa_header.pixel_size != 32 && targa_header.pixel_size != 24) && targa_header.image_type != 3)
	{
		throw ImageException("LoadTGA: Only 32 or 24 bit images supported (no colormaps)");
	}

	columns = targa_header.width;
	rows = targa_header.height;
	numPixels = columns * rows * 4;

	if (!columns || !rows || numPixels > 0x7FFFFFFF || numPixels / columns / 4 != rows)
	{
		throw ImageException("LoadTGA: %s has an invalid image size", name);
	}

	targa_rgba = new uint8_t[numPixels];

	if (targa_header.id_length != 0)
	{
		if (buf_p + targa_header.id_length > end)
		{
			delete[] targa_rgba;
			throw ImageException("LoadTGA: header too short (%s)", name);
		}

		buf_p += targa_header.id_length;  // skip TARGA image comment
	}

	if (targa_header.image_type == 2 || targa_header.image_type == 3)
	{
		if (buf_p + columns * rows*targa_header.pixel_size / 8 > end)
		{
			delete[] targa_rgba;
			throw ImageException("LoadTGA: file truncated %s", name);
		}

		// Uncompressed RGB or gray scale image
		for (row = rows - 1; row >= 0; row--)
		{
			pixbuf = targa_rgba + row * columns * 4;
			for (column = 0; column < columns; column++)
			{
				unsigned char red, green, blue, alphabyte;
				switch (targa_header.pixel_size)
				{

				case 8:
					blue = *buf_p++;
					green = blue;
					red = blue;
					*pixbuf++ = red;
					*pixbuf++ = green;
					*pixbuf++ = blue;
					*pixbuf++ = 255;
					break;

				case 24:
					blue = *buf_p++;
					green = *buf_p++;
					red = *buf_p++;
					*pixbuf++ = red;
					*pixbuf++ = green;
					*pixbuf++ = blue;
					*pixbuf++ = 255;
					break;
				case 32:
					blue = *buf_p++;
					green = *buf_p++;
					red = *buf_p++;
					alphabyte = *buf_p++;
					*pixbuf++ = red;
					*pixbuf++ = green;
					*pixbuf++ = blue;
					*pixbuf++ = alphabyte;
					break;
				default:
					delete[] targa_rgba;
					throw ImageException("LoadTGA: illegal pixel_size '%d' in file '%s'", targa_header.pixel_size, name);
					break;
				}
			}
		}
	}
	else if (targa_header.image_type == 10) {   // Runlength encoded RGB images
		unsigned char red, green, blue, alphabyte, packetHeader, packetSize, j;

		for (row = rows - 1; row >= 0; row--) {
			pixbuf = targa_rgba + row * columns * 4;
			for (column = 0; column < columns; ) {
				if (buf_p + 1 > end)
				{
					delete[] targa_rgba;
					throw ImageException("LoadTGA: file truncated %s", name);
				}
				packetHeader = *buf_p++;
				packetSize = 1 + (packetHeader & 0x7f);
				if (packetHeader & 0x80) {        // run-length packet
					if (buf_p + targa_header.pixel_size / 8 > end)
					{
						delete[] targa_rgba;
						throw ImageException("LoadTGA: file truncated %s", name);
					}
					switch (targa_header.pixel_size) {
					case 24:
						blue = *buf_p++;
						green = *buf_p++;
						red = *buf_p++;
						alphabyte = 255;
						break;
					case 32:
						blue = *buf_p++;
						green = *buf_p++;
						red = *buf_p++;
						alphabyte = *buf_p++;
						break;
					default:
						delete[] targa_rgba;
						throw ImageException("LoadTGA: illegal pixel_size '%d' in file '%s'", targa_header.pixel_size, name);
						break;
					}

					for (j = 0; j < packetSize; j++) {
						*pixbuf++ = red;
						*pixbuf++ = green;
						*pixbuf++ = blue;
						*pixbuf++ = alphabyte;
						column++;
						if (column == columns) { // run spans across rows
							column = 0;
							if (row > 0)
								row--;
							else
								goto breakOut;
							pixbuf = targa_rgba + row * columns * 4;
						}
					}
				}
				else {                            // non run-length packet

					if (buf_p + targa_header.pixel_size / 8 * packetSize > end)
					{
						delete[] targa_rgba;
						throw ImageException("LoadTGA: file truncated %s", name);
					}
					for (j = 0; j < packetSize; j++) {
						switch (targa_header.pixel_size) {
						case 24:
							blue = *buf_p++;
							green = *buf_p++;
							red = *buf_p++;
							*pixbuf++ = red;
							*pixbuf++ = green;
							*pixbuf++ = blue;
							*pixbuf++ = 255;
							break;
						case 32:
							blue = *buf_p++;
							green = *buf_p++;
							red = *buf_p++;
							alphabyte = *buf_p++;
							*pixbuf++ = red;
							*pixbuf++ = green;
							*pixbuf++ = blue;
							*pixbuf++ = alphabyte;
							break;
						default:
							delete[] targa_rgba;
							throw ImageException("LoadTGA: illegal pixel_size '%d' in file '%s'", targa_header.pixel_size, name);
							break;
						}
						column++;
						if (column == columns) { // pixel packet run spans across rows
							column = 0;
							if (row > 0)
								row--;
							else
								goto breakOut;
							pixbuf = targa_rgba + row * columns * 4;
						}
					}
				}
			}
		breakOut:;
		}
	}

#if 1
	// TTimo: this is the chunk of code to ensure a behavior that meets TGA specs 
	// bit 5 set => top-down
	if (targa_header.attributes & 0x20)
	{
		uint8_t *flip = new uint8_t[columns * 4];
		uint8_t *src, *dst;
		for (row = 0; row < rows / 2; row++)
		{
			src = targa_rgba + row * 4 * columns;
			dst = targa_rgba + (rows - row - 1) * 4 * columns;
			memcpy(flip, src, columns * 4);
			memcpy(src, dst, columns * 4);
			memcpy(dst, flip, columns * 4);
		}
		delete[] flip;
	}
#else
	// instead we just print a warning
	if (targa_header.attributes & 0x20)
	{
		ri.Printf(PRINT_WARNING, "WARNING: '%s' TGA file header declares top-down image, ignoring\n", name);
	}
#endif

	data = targa_rgba;
	dataSize = numPixels;
	width = columns;
	height = rows;
	pixelFormat = PixelFormat::IF_RGBA;
}
