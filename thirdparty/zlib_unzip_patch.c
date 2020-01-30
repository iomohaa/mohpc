#include "zlib/contrib/minizip/unzip.c"

extern int ZEXPORT unzseek64(unzFile file, ZPOS64_T offset, int origin);

extern int ZEXPORT unzseek(unzFile file, uLong offset, int origin)
{
	return unzseek64(file, offset, origin);
}

extern int ZEXPORT unzseek64(unzFile file, ZPOS64_T offset, int origin)
{
	unz64_s *s = NULL;
	ZPOS64_T stream_pos_begin = 0;
	ZPOS64_T stream_pos_end = 0;
	ZPOS64_T position = 0;
	int is_within_buffer = 0;

	if (file == NULL)
		return UNZ_PARAMERROR;
	s = (unz64_s*)file;

	if (s->pfile_in_zip_read == NULL)
		return UNZ_ERRNO;
	if (s->pfile_in_zip_read->compression_method != 0)
	{
		ZPOS64_T new_origin;
		ZPOS64_T current_origin;
		ZPOS64_T amount;
		unz64_s* s = file;
		char data[32000];

		current_origin = unztell(file);
		new_origin = current_origin;

		switch (origin)
		{
		case SEEK_CUR:
			new_origin = offset + current_origin;
			break;
		case SEEK_SET:
			new_origin = offset;
			break;
		case SEEK_END:
			new_origin = s->cur_file_info.uncompressed_size - offset;
			break;
		}

		if (new_origin < current_origin)
		{
			unzOpenCurrentFile(file);
			current_origin = 0;
		}

		for (position = current_origin; position < new_origin; position += amount)
		{
			amount = new_origin - position;

			if (new_origin - position > sizeof(data))
			{
				amount = sizeof(data);
			}

			unzReadCurrentFile(file, data, (unsigned int)amount);
		}

		return UNZ_OK;
	}

	if (origin == SEEK_SET)
		position = offset;
	else if (origin == SEEK_CUR)
		position = s->pfile_in_zip_read->total_out_64 + offset;
	else if (origin == SEEK_END)
		position = s->cur_file_info.compressed_size + offset;
	else
		return UNZ_PARAMERROR;

	if (position > s->cur_file_info.compressed_size)
		return UNZ_PARAMERROR;

	stream_pos_end = s->pfile_in_zip_read->pos_in_zipfile;
	stream_pos_begin = stream_pos_end;

	if (stream_pos_begin > UNZ_BUFSIZE)
		stream_pos_begin -= UNZ_BUFSIZE;
	else
		stream_pos_begin = 0;

	is_within_buffer =
		(s->pfile_in_zip_read->stream.avail_in != 0) &&
		(s->pfile_in_zip_read->rest_read_compressed != 0 || s->cur_file_info.compressed_size < UNZ_BUFSIZE) &&
		(position >= stream_pos_begin && position < stream_pos_end);

	if (is_within_buffer)
	{
		s->pfile_in_zip_read->stream.next_in += position - s->pfile_in_zip_read->total_out_64;
		s->pfile_in_zip_read->stream.avail_in = (uInt)(stream_pos_end - position);
	}
	else
	{
		s->pfile_in_zip_read->stream.avail_in = 0;
		s->pfile_in_zip_read->stream.next_in = 0;

		s->pfile_in_zip_read->pos_in_zipfile = s->pfile_in_zip_read->offset_local_extrafield + position;
		s->pfile_in_zip_read->rest_read_compressed = s->cur_file_info.compressed_size - position;
	}

	s->pfile_in_zip_read->rest_read_uncompressed -= (position - s->pfile_in_zip_read->total_out_64);
	s->pfile_in_zip_read->stream.total_out = (uLong)position;
	s->pfile_in_zip_read->total_out_64 = position;

	return UNZ_OK;
}
