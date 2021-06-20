#include <Shared.h>
#include <MOHPC/Assets/Formats/Sound.h>
#include <MOHPC/Assets/Managers/AssetManager.h>
#include <MOHPC/Files/Managers/FileManager.h>
#include <MOHPC/Utility/Misc/Endian.h>

//#include <lame/lame.h>
#include <libmad/mad.h>
#include <sstream>
#include <fstream>
#include <cstring>

using namespace MOHPC;

static void write_16_bits_low_high(std::stringstream& out, int val)
{
	unsigned char bytes[2];
	bytes[0] = (val & 0xff);
	bytes[1] = ((val >> 8) & 0xff);
	out.write((const char *)bytes, 2);
}

static void write_32_bits_low_high(std::stringstream& out, int val)
{
	unsigned char bytes[4];
	bytes[0] = (val & 0xff);
	bytes[1] = ((val >> 8) & 0xff);
	bytes[2] = ((val >> 16) & 0xff);
	bytes[3] = ((val >> 24) & 0xff);
	out.write((const char *)bytes, 4);
}

static void WriteWaveHeader(std::stringstream& out, int pcmbytes, int freq, int channels, int bits)
{
	int     bytes = (bits + 7) / 8;

	/* quick and dirty, but documented */
	out.write("RIFF", 4); /* label */
	write_32_bits_low_high(out, pcmbytes + 44 - 8); /* length in bytes without header */
	out.write("WAVEfmt ", 4 * 2); /* 2 labels */
	write_32_bits_low_high(out, 2 + 2 + 4 + 4 + 2 + 2); /* length of PCM format declaration area */
	write_16_bits_low_high(out, 1); /* is PCM? */
	write_16_bits_low_high(out, channels); /* number of channels */
	write_32_bits_low_high(out, freq); /* sample frequency in [Hz] */
	write_32_bits_low_high(out, freq * channels * bytes); /* bytes per second */
	write_16_bits_low_high(out, channels * bytes); /* bytes per sample time */
	write_16_bits_low_high(out, bits); /* bits per sample */
	out.write("data", 4); /* label */
	write_32_bits_low_high(out, pcmbytes); /* length in bytes of raw PCM data */
}

struct buffer_s {
	unsigned char const *start;
	uint32_t length;
	std::stringstream stream;
	uint32_t numSamples;
	int32_t numChannels;
	int32_t sampleRate;
};

static
enum mad_flow input(void *data,
	struct mad_stream *stream)
{
	buffer_s *buffer = (buffer_s*)data;

	if (!buffer->length)
		return MAD_FLOW_STOP;

	mad_stream_buffer(stream, buffer->start, buffer->length);

	buffer->length = 0;

	return MAD_FLOW_CONTINUE;
}

static inline
signed int scale(mad_fixed_t sample)
{
	/* round */
	sample += (1L << (MAD_F_FRACBITS - 16));

	/* clip */
	if (sample >= MAD_F_ONE)
		sample = MAD_F_ONE - 1;
	else if (sample < -MAD_F_ONE)
		sample = -MAD_F_ONE;

	/* quantize */
	return sample >> (MAD_F_FRACBITS + 1 - 16);
}

static
enum mad_flow output(void *data,
	struct mad_header const *header,
	struct mad_pcm *pcm)
{
	buffer_s *buffer = (buffer_s*)data;
	unsigned int nchannels, nsamples;
	mad_fixed_t const *left_ch, *right_ch;

	/* pcm->samplerate contains the sampling frequency */

	nchannels = pcm->channels;
	nsamples = pcm->length;
	left_ch = pcm->samples[0];
	right_ch = pcm->samples[1];

	if (!buffer->stream.tellp())
	{
		buffer->numChannels = nchannels;
		buffer->numSamples = 0;
		buffer->sampleRate = header->samplerate;
		WriteWaveHeader(buffer->stream, 0x7FFFFFFF, header->samplerate, nchannels, 16);
	}

	buffer->numSamples += nsamples;

	if (nsamples && nchannels)
	{
		size_t datasize = nsamples * sizeof(short) * nchannels;
		uint8_t* data = new uint8_t[datasize];
		uint8_t* datap = data;

		if (nchannels == 2)
		{
			while (nsamples--)
			{
				signed int sample;

				/* output sample(s) in 16-bit signed little-endian PCM */

				sample = scale(*left_ch++);

				unsigned short value = short(Endian.LittleLong(sample));
				*(short*)datap = value;
				datap += sizeof(short);

				sample = scale(*right_ch++);

				value = short(Endian.LittleLong(sample));
				*(short*)datap = value;
				datap += sizeof(short);
			}
		}
		else
		{
			while (nsamples--)
			{
				signed int sample;

				/* output sample(s) in 16-bit signed little-endian PCM */

				sample = scale(*left_ch++);

				unsigned short value = short(Endian.LittleLong(sample));
				*(short*)datap = value;
				datap += sizeof(short);
			}
		}

		buffer->stream.write((const char *)data, datasize);
		delete[] data;
	}

	return MAD_FLOW_CONTINUE;
}

static
enum mad_flow error(void *data,
	struct mad_stream *stream,
	struct mad_frame *frame)
{
	buffer_s *buffer = (buffer_s*)data;

	fprintf(stderr, "decoding error 0x%04x (%s) at byte offset %zd\n",
		stream->error, mad_stream_errorstr(stream),
		stream->this_frame - buffer->start);

	/* return MAD_FLOW_BREAK here to stop decoding (and propagate an error) */

	return MAD_FLOW_CONTINUE;
}

static
int decode(unsigned char const *start, unsigned long length)
{
	buffer_s buffer;
	struct mad_decoder decoder;
	int result;

	/* initialize our private message structure */

	buffer.start = start;
	buffer.length = length;

	/* configure input, output, and error functions */

	mad_decoder_init(&decoder, &buffer,
		input, 0 /* header */, 0 /* filter */, output,
		error, 0 /* message */);

	/* start decoding */

	result = mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC);

	/* release the decoder */

	mad_decoder_finish(&decoder);

	return result;
}

MOHPC_OBJECT_DEFINITION(Sound);
Sound::Sound()
{
	data = nullptr;
	dataLen = 0;
}

Sound::~Sound()
{
	if (data)
	{
		delete[] data;
	}
}

void Sound::Load()
{
	FileManager* fileMan = GetFileManager();
	const char* fname = GetFilename().c_str();

	FilePtr File = fileMan->OpenFile(fname);
	if (!File) {
		throw AssetError::AssetNotFound(GetFilename());
	}

	void *buf = nullptr;
	dataLen = (size_t)File->ReadBuffer(&buf);
	if (dataLen <= 0)
	{
		// empty sound, considered as valid
		return;
	}

	const char* ext = fileMan->GetFileExtension(fname);
	if (!str::icmp(ext, "mp3"))
	{
		DecodeLAME(buf, dataLen);
	}
	else if (!stricmp(ext, "wav"))
	{
		data = new uint8_t[dataLen];
		memcpy(data, buf, dataLen);
	}
	else {
		throw SoundError::BadOrUnsupportedSound(ext);
	}
}

void Sound::DecodeLAME(void *buf, uint64_t len)
{
#if 0
	lame_t lame = lame_init();
	lame_set_decode_only(lame, 1);
	if (lame_init_params(lame) == -1)
	{
		return false;
	}

	hip_t hip = hip_decode_init();

	mp3data_struct mp3data;
	memset(&mp3data, 0, sizeof(mp3data));

	int32_t nChannels = -1;
	int32_t nSampleRate = -1;
	uint64_t mp3_len;
	uint32_t wavsize = 0;
	int32_t samples = 0;
	uint8_t* mp3_buffer = (uint8_t*)buf;

	static const int32_t pcmSize = 4096;
	static const int32_t mp3Size = 4096;

	short pcm_l_r[pcmSize * 2];

	uint64_t prevLen = len;
	uint64_t remainingLength = len - 4096;
	if (remainingLength < 0)
	{
		remainingLength = 0;
	}

	std::stringstream wavdata;

	uint64_t readLen = prevLen - remainingLength;
	while (readLen > 0)
	{
		mp3_len = readLen;

		do
		{
			samples = hip_decode1_headers(hip, mp3_buffer, mp3_len, pcm_l_r, pcm_l_r + pcmSize, &mp3data);
			wavsize += samples;

			if (mp3data.header_parsed == 1)
			{
				if (nChannels < 0)
				{
					WriteWaveHeader(wavdata, 0x7FFFFFFF, mp3data.samplerate, mp3data.stereo, 16);
				}
				nChannels = mp3data.stereo;
				nSampleRate = mp3data.samplerate;
			}

			if (samples > 0 && mp3data.header_parsed != 1)
			{
				hip_decode_exit(hip);
				lame_close(lame);
				return false;
			}

			for (int32_t i = 0; i < samples; i++)
			{
				wavdata.write((const char *)&pcm_l_r[i], sizeof(short));
				if (nChannels == 2)
				{
					wavdata.write((const char *)&pcm_l_r[pcmSize + i], sizeof(short));
				}
			}
			mp3_len = 0;
		} while (samples > 0);

		mp3_buffer += readLen;

		prevLen = remainingLength;
		remainingLength -= 4096;
		if (remainingLength < 0)
		{
			remainingLength = 0;
		}
		readLen = prevLen - remainingLength;
	}

	uint32_t dataSize = 0;
	uint32_t i = (16 / 8) * mp3data.stereo;
	if (wavsize <= 0)
	{
		dataSize = 0;
	}
	else if (wavsize > 0xFFFFFFD0 / i)
	{
		dataSize = 0xFFFFFFD0;
	}
	else
	{
		dataSize = wavsize * i;
	}

	wavdata.seekp(0, wavdata.beg);
	WriteWaveHeader(wavdata, dataSize, mp3data.samplerate, mp3data.stereo, 16);
	wavdata.seekp(0, wavdata.end);

	if (wavdata.good())
	{
		dataLen = wavdata.tellp();
		data = new uint8_t[dataLen];
		wavdata.read((char*)data, dataLen);
	}

	hip_decode_exit(hip);
	lame_close(lame);
#else
	buffer_s buffer;
	struct mad_decoder decoder;
	int result;

	/* initialize our private message structure */
	buffer.start = (const unsigned char*)buf;
	buffer.length = (uint32_t)len;

	/* configure input, output, and error functions */
	mad_decoder_init(&decoder, &buffer,
		input, 0 /* header */, 0 /* filter */, output,
		error, 0 /* message */);

	/* start decoding */
	result = mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC);

	/* release the decoder */
	mad_decoder_finish(&decoder);

	if (result != -1)
	{
		uint32_t dataSize = 0;
		uint32_t i = (16 / 8) * buffer.numChannels;
		if (buffer.numSamples <= 0)
		{
			dataSize = 0;
		}
		else if (buffer.numSamples > 0xFFFFFFD0 / i)
		{
			dataSize = 0xFFFFFFD0;
		}
		else
		{
			dataSize = buffer.numSamples * i;
		}

		buffer.stream.seekp(0, buffer.stream.beg);
		WriteWaveHeader(buffer.stream, dataSize, buffer.sampleRate, buffer.numChannels, 16);
		buffer.stream.seekp(0, buffer.stream.end);

		if (buffer.stream.good())
		{
			dataLen = (size_t)buffer.stream.tellp();
			data = new uint8_t[dataLen];
			buffer.stream.read((char*)data, dataLen);
		}
	}
#endif
}

uint8_t * Sound::GetData() const
{
	return data;
}

size_t Sound::GetDataLength() const
{
	return dataLen;
}

SoundError::BadOrUnsupportedSound::BadOrUnsupportedSound(const str& inExtension)
	: extension(inExtension)
{
}

const char* SoundError::BadOrUnsupportedSound::getExtension() const
{
	return extension.c_str();
}

const char* SoundError::BadOrUnsupportedSound::what() const noexcept
{
	return "Bad or unsupported sound format";
}
