////////////////////////////////////////////////////////////////////////////////
//    Scorched3D (c) 2000-2011
//
//    This file is part of Scorched3D.
//
//    Scorched3D is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    Scorched3D is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License along
//    with this program; if not, write to the Free Software Foundation, Inc.,
//    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <math.h>

#ifdef __cplusplus
extern "C" { 
#endif /* __cplusplus */

#ifdef __DARWIN__
#include <UnixImageIO/jpeglib.h>
#else
#include <jpeglib.h>
#endif

#ifdef __cplusplus
}
#endif

#include <common/Defines.h>
#include <image/ImageJpgFactory.h>

Image ImageJpgFactory::loadFromFile(const char *filename, const char *alphafilename, bool invert)
{
	Image result;
	Image bitmap = loadFromFile(filename, false);
	Image alpha = loadFromFile(alphafilename, false);

	if (bitmap.getBits() && alpha.getBits() && 
		bitmap.getWidth() == alpha.getWidth() &&
		bitmap.getHeight() == alpha.getHeight())
	{
		result = Image(bitmap.getWidth(), bitmap.getHeight(), true);
		result.setLossless(false);

		unsigned char *bbits = bitmap.getBits();
		unsigned char *abits = alpha.getBits();
		unsigned char *bits = result.getBits();
		for (int y=0; y<bitmap.getHeight(); y++)
		{
			for (int x=0; x<bitmap.getWidth(); x++)
			{
				bits[0] = bbits[0];
				bits[1] = bbits[1];
				bits[2] = bbits[2];

				unsigned char avg = (unsigned char)(int(abits[0] + abits[1] + abits[2]) / 3);
				if (invert)
				{
					bits[3] = (unsigned char)(255 - avg);
				}
				else
				{
					bits[3] = avg;
				}

				bbits += 3;
				abits += 3;
				bits += 4;
			}
		}
	}

	return result;
}

Image ImageJpgFactory::loadFromFile(const char * filename, bool readalpha)
{
	FILE *file = fopen(filename, "rb");
	if (!file) return Image();

	int read = 0;
	char buffer[256];
	NetBuffer netBuffer;
	while (read = (int) fread(buffer, 1, 256, file))
	{
		netBuffer.addDataToBuffer(buffer, read);
	}
	fclose(file);

	return loadFromBuffer(netBuffer, readalpha);
}

METHODDEF(void)
init_source (j_decompress_ptr cinfo)
{
}

METHODDEF(boolean)
fill_input_buffer (j_decompress_ptr cinfo)
{
	struct jpeg_source_mgr * src = cinfo->src;
	static JOCTET FakeEOI[] = { 0xFF, JPEG_EOI };

	/* Insert a fake EOI marker */
	src->next_input_byte = FakeEOI;
	src->bytes_in_buffer = 2;

	return TRUE;
}

METHODDEF(void)
skip_input_data (j_decompress_ptr cinfo, long num_bytes)
{
	struct jpeg_source_mgr * src = cinfo->src;

	if(num_bytes >= (long)src->bytes_in_buffer)
	{
		fill_input_buffer(cinfo);
		return;
	}

	src->bytes_in_buffer -= num_bytes;
	src->next_input_byte += num_bytes;
}

METHODDEF(void)
term_source (j_decompress_ptr cinfo)
{
  /* no work necessary here */
}

Image ImageJpgFactory::loadFromBuffer(NetBuffer &buffer, bool readalpha)
{
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;

	cinfo.err = jpeg_std_error(&jerr);	
	jpeg_create_decompress(&cinfo);

	// Get the buffer reading entry points
	if (!cinfo.src)
	{
		cinfo.src = (struct jpeg_source_mgr *)
			(*cinfo.mem->alloc_small) ((j_common_ptr) &cinfo, JPOOL_PERMANENT,
				sizeof(struct jpeg_source_mgr));
	}
	struct jpeg_source_mgr *src = cinfo.src;

	// Set up buffer reading functions
	src->init_source = init_source;
	src->fill_input_buffer = fill_input_buffer;
	src->skip_input_data = skip_input_data;
	src->resync_to_restart = jpeg_resync_to_restart; /* use default method */
	src->term_source = term_source;

	// Set up buffer
	src->bytes_in_buffer = buffer.getBufferUsed();
	src->next_input_byte = (JOCTET *) buffer.getBuffer();

	jpeg_read_header(&cinfo, TRUE);
	jpeg_start_decompress(&cinfo);

	Image result;
	if ((cinfo.output_components == 3 && !readalpha) ||
		(cinfo.output_components == 4 && readalpha))
	{
		result = Image(cinfo.output_width, cinfo.output_height, readalpha);
		result.setLossless(false);

		JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)
			((j_common_ptr) &cinfo, JPOOL_IMAGE, 
				cinfo.output_width * cinfo.output_components, 1);

		while (cinfo.output_scanline < cinfo.output_height)
		{
			int currentLine = cinfo.output_scanline;
			int lines = jpeg_read_scanlines(&cinfo, buffer, 1);

			if (lines > 0)
			{
				int destPos = cinfo.output_width * cinfo.output_components * 
					(cinfo.output_height - currentLine - 1);
				JSAMPLE *src = buffer[0];
				unsigned char *dest = &result.getBits()[destPos];

				for (unsigned int i = 0; i < cinfo.output_width; ++i, 
					dest+=cinfo.output_components, src+=cinfo.output_components)
				{
					dest[0] = src[0];
					dest[1] = src[1];
					dest[2] = src[2];
					if (readalpha) dest[3] = src[3];
				}
			}
		}
	}

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	return result;
}
