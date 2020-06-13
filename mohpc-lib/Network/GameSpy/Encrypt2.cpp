/*

GS enctype2 servers list decoder 0.1.1a
by Luigi Auriemma
e-mail: aluigi@autistici.org
web:    aluigi.org


INTRODUCTION
============
This is the algorithm used to decrypt the data sent by the Gamespy
master server (or any other compatible server) using the enctype 2
method.


USAGE
=====
Add the following prototype at the beginning of your code:

  uint8_t *enctype2_decoder(uint8_t *, uint8_t *, int *);

then use:

        pointer = enctype2_decoder(
            gamekey,        // the gamekey
            buffer,         // all the data received from the master server
            &buffer_len);   // the size of the master server

The return value is a pointer to the decrypted zone of buffer and
buffer_len is modified with the size of the decrypted data.

A simpler way to use the function is just using:

  len = enctype2_wrapper(key, data, data_len);


THANX TO
========
REC (http://www.backerstreet.com/rec/rec.htm) which has helped me in many
parts of the code.


LICENSE
=======
    Copyright 2004,2005,2006,2007,2008 Luigi Auriemma

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

    http://www.gnu.org/licenses/gpl.txt

*/

#include <string.h>
#include <MOHPC/Network/GameSpy/Encryption.h>

namespace MOHPC
{
	using namespace Network;
	using namespace SharedEncryption;

	size_t EncryptionLevel2::decode(const uint8_t* key, uint8_t* data, size_t size)
	{
		*data ^= 0xec;

		uint8_t* datap = data + 1;

		for (size_t i = 0; key[i]; i++) {
			datap[i] ^= key[i];
		}

		uint32_t dest[326];
		memset(dest, 0, sizeof(dest));
		const size_t destSize = *data;

		if (destSize) {
			encshare4(datap, destSize, dest);
		}

		datap += destSize;
		size -= destSize + 1;
		if (size < 6)
		{
			memmove(data, datap, size);
			return 0;
		}

		encshare1(dest, datap, size);

		size -= 6;
		memmove(data, datap, size);
		return size;
	}

	size_t EncryptionLevel2::encode(const uint8_t* key, uint8_t* data, size_t size)
	{
		uint32_t dest[326];
		uint8_t* datap;
		const size_t header_size = 8;

		for (size_t i = size - 1; i >= 0; i--) {
			data[1 + header_size + i] = data[i];
		}
		*data = header_size;

		datap = data + 1;
		memset(datap, 0, *data);

		for (size_t i = 256; i < 326; i++) dest[i] = 0;
		encshare4(datap, *data, dest);

		memset(data + 1 + *data + size, 0, 6);
		encshare1(dest, datap + *data, size + 6);

		for (size_t i = 0; key[i]; i++) datap[i] ^= key[i];
		size += 1 + *data + 6;
		*data ^= 0xec;
		return size;
	}
}

