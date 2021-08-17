#pragma once

#include <streambuf>
#include <zlib/contrib/minizip/unzip.h>
#include <algorithm>
#include <iostream>
#include <cstring>

#undef min

// function from patched unzip
extern "C" int ZEXPORT unzseek64(unzFile file, ZPOS64_T offset, int origin);

template<
	typename char_type,
	typename traits_type = std::char_traits<char_type>>
class basic_decompression_buf : public std::basic_streambuf<char_type, traits_type>
{
	enum
	{
		BUFFER_SIZE = 10 * 1024
	};

public:
	typedef typename traits_type::int_type 		int_type;
	typedef typename traits_type::pos_type 		pos_type;
	typedef typename traits_type::off_type 		off_type;

	basic_decompression_buf(unzFile File, const unz_file_pos& Position, size_t size)
	{
		m_offset = 0;
		m_roffset = 0;
		m_file = File;
		m_pos = Position;
		m_size = size;
	}

	~basic_decompression_buf()
	{
	}

	virtual pos_type seekpos(pos_type pt,
		std::ios::openmode =
		(std::ios::openmode)(std::ios::in | std::ios::out))
	{
		SetCurrentFile();
#ifdef __WIN32
		unzseek64(m_file, pt.seekpos(), (int)(std::streamoff(pt) - pt.seekpos()));
#else
		unzseek64(m_file, std::streamoff(pt), 1);
#endif
		m_offset = unztell(m_file);
		this->setg(NULL, NULL, NULL);
		return pt;
	}

	virtual pos_type seekoff(off_type ot,
		std::ios::seekdir sd,
		std::ios::openmode =
		(std::ios::openmode)(std::ios::in | std::ios::out))
	{
		SetCurrentFile();

		if (ot == 0 && sd == std::ios::cur)
		{
			return unztell64(m_file);
		}
		else
		{
			off_type realoff;
			switch (sd)
			{
			default:
			case std::ios::beg:
				realoff = ot;
				break;
			case std::ios::cur:
				realoff = m_offset + ot;
				break;
			case std::ios::end:
				realoff = m_size;
				break;
			}

			if (realoff < m_offset || this->eback() + realoff > this->egptr())
			{
				unzseek64(m_file, ot, sd);
				m_offset = unztell64(m_file);
				this->setg(NULL, NULL, NULL);
			}
			else
			{
				//setg(m_readbuffer, m_readbuffer, m_readbuffer + m_offset);
				this->setg(this->eback(), this->eback() + realoff, this->egptr());
			}

			return ot;
		}
	}

	int_type underflow()
	{
		size_t putBackSize = 0;
		if (this->eback() != 0)
		{
			const size_t minPutBack = BUFFER_SIZE / 2;
			putBackSize = std::min(minPutBack, (size_t)(this->gptr() - this->eback()));

			memmove(m_readbuffer, this->gptr() - putBackSize, putBackSize);
		}

		char_type* begin = m_readbuffer + putBackSize;

		SetCurrentFile();

		m_roffset = unztell64(m_file);
		int readBytes = unzReadCurrentFile(m_file, begin, (unsigned int)(BUFFER_SIZE - putBackSize) * sizeof(char_type));
		m_offset = m_roffset + readBytes;

		if (readBytes > 0)
		{
			this->setg(begin, begin, begin + readBytes);
		}

		if (readBytes > 0)
		{
			return this->sgetc();
		}
		else
		{
			return traits_type::eof();
		}
	}

private:
	void SetCurrentFile()
	{
		unz_file_pos pos;
		unzGetFilePos(m_file, &pos);

		if (pos.num_of_file != m_pos.num_of_file || pos.pos_in_zip_directory != m_pos.pos_in_zip_directory)
		{
			unzGoToFilePos(m_file, &m_pos);
			unzOpenCurrentFile(m_file);
		}

		off_type currentOff = unztell64(m_file);

		if(currentOff >= 0 && currentOff != m_offset)
		{
			unzseek64(m_file, m_offset, 0);
		}
	}

	unzFile m_file;
	unz_file_pos m_pos;
	size_t m_size;
	off_type m_roffset;
	off_type m_offset;
	char_type m_readbuffer[BUFFER_SIZE];
};
