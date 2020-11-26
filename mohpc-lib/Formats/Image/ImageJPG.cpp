#include <Shared.h>
#include <MOHPC/Formats/Image.h>
#include <jpeg-9d/jpeglib.h>

using namespace MOHPC;

void Image::LoadJPEG(const char *name, void *buf, uint64_t len)
{
	/* This struct contains the JPEG decompression parameters and pointers to
	* working space (which is allocated as needed by the JPEG library).
	*/
	struct jpeg_decompress_struct cinfo;

	/* This struct represents a JPEG error handler.  It is declared separately
	 * because applications often want to supply a specialized error handler
	 * (see the second half of this file for an example).  But here we just
	 * take the easy way out and use the standard error handler, which will
	 * print a message on stderr and call exit() if compression fails.
	 * Note that this struct must live as long as the main JPEG parameter
	 * struct, to avoid dangling-pointer problems.
	 */
	struct jpeg_error_mgr jerr;

	cinfo.err = nullptr;

	/* Step 1: initialize the JPEG decompression object. */

	/* We have to set up the error handler first, in case the initialization
	 * step fails.  (Unlikely, but it could happen if you are out of memory.)
	 * This routine fills in the contents of struct jerr, and returns jerr's
	 * address which we place into the link field in cinfo.
	 */
	cinfo.err = jpeg_std_error(&jerr);
	cinfo.err->error_exit =
	[](j_common_ptr comInfo)
	{
		throw ImageError::BadImage();
	};

	cinfo.err->output_message =
	[](j_common_ptr cinfo)
	{
	};

	try
	{
		jpeg_create_decompress(&cinfo);

		/* Step 2: specify data source (eg, a file) */
		jpeg_mem_src(&cinfo, (const unsigned char*)buf, (unsigned long)len);

		/* Step 3: read file parameters with jpeg_read_header() */
		jpeg_read_header(&cinfo, (boolean)true);

		/* Step 4: set parameters for decompression */

		/*
		* Make sure it always converts images to RGB color space. This will
		* automatically convert 8-bit greyscale images to RGB as well.
		*/
		cinfo.out_color_space = JCS_RGB;

		/* Step 5: Start decompressor */

		jpeg_start_decompress(&cinfo);

		width = cinfo.output_width;
		height = cinfo.output_height;

		const int32_t pixelcount = width * height;

		const uint32_t memcount = pixelcount * 4;
		const uint32_t row_stride = cinfo.output_width * cinfo.output_components;

		data = new uint8_t[memcount];
		dataSize = memcount;

		uint8_t* rbuf;

		/* Step 6: while (scan lines remain to be read) */
		/*           jpeg_read_scanlines(...); */

		/* Here we use the library's state variable cinfo.output_scanline as the
		 * loop counter, so that we don't have to keep track ourselves.
		 */
		while (cinfo.output_scanline < cinfo.output_height)
		{
			/* jpeg_read_scanlines expects an array of pointers to scanlines.
			 * Here the array is only one element long, but you could ask for
			 * more than one scanline at a time if that's more convenient.
			 */
			rbuf = ((data + (row_stride * cinfo.output_scanline)));
			jpeg_read_scanlines(&cinfo, &rbuf, 1);
		}

		rbuf = data;

		/* Expand from RGB to RGBA */
		uintptr_t sindex = pixelcount * cinfo.output_components;
		uintptr_t dindex = memcount;

		do
		{
			rbuf[--dindex] = 255;
			rbuf[--dindex] = rbuf[--sindex];
			rbuf[--dindex] = rbuf[--sindex];
			rbuf[--dindex] = rbuf[--sindex];
		} while (sindex);

		/* Step 7: Finish decompression */

		jpeg_finish_decompress(&cinfo);
		/* We can ignore the return value since suspension is not possible
		 * with the stdio data source.
		 */
	}
	catch (...)
	{
		jpeg_destroy_decompress(&cinfo);
		throw;
	}

	/* Step 8: Release JPEG decompression object */

	/* This is an important step since it will release a good deal of memory. */
	jpeg_destroy_decompress(&cinfo);

	pixelFormat = PixelFormat::RGB;
}
