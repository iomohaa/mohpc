#pragma once

#include <stdint.h>

namespace MOHPC
{
	namespace Network
	{
		class IEncryption
		{
		public:
			/**
			 * Decode data using the specified key.
			 *
			 * @param	key		Key to use for decoding.
			 * @param	data	Data to decode.
			 * @param	size	Size of the data.
			 */
			virtual size_t decode(const uint8_t* key, uint8_t* data, size_t size) = 0;

			/**
			 * Encode data using the specified key.
			 *
			 * @param	key		Key to use for encoding.
			 * @param	data	Data to decode.
			 * @param	size	Size of the data.
			 */
			virtual size_t encode(const uint8_t* key, uint8_t* data, size_t size) = 0;
		};

		class EncryptionLevel1 : public IEncryption
		{
		private:
			uint8_t enc1key[261];

		public:
			virtual size_t decode(const uint8_t* key, uint8_t* data, size_t size) override;
			virtual size_t encode(const uint8_t* key, uint8_t* data, size_t size) override;

		private:
			void func1(uint8_t* id, int idlen);
			void func2(uint8_t* data, int size, uint8_t* crypt);
			void func3(uint8_t* data, size_t len, uint8_t* buff);
			void func4(const uint8_t* id, size_t idlen);
			int func5(int cnt, const uint8_t* id, size_t idlen, size_t* n1, size_t* n2);
			void func6(uint8_t* data, size_t len);
			int func7(size_t len);
			void func8(uint8_t* data, size_t len, const uint8_t* enctype1_data);
		};

		class EncryptionLevel2 : public IEncryption
		{
		public:
			virtual size_t decode(const uint8_t* key, uint8_t* data, size_t size) override;
			virtual size_t encode(const uint8_t* key, uint8_t* data, size_t size) override;
		};

		namespace SharedEncryption
		{
			void encshare2(uint32_t* tbuff, uint32_t* tbuffp, size_t len);
			void encshare1(uint32_t* tbuff, uint8_t* datap, size_t len);
			void encshare3(uint32_t* data, int n1, int n2);
			void encshare4(uint8_t* src, size_t size, uint32_t* dest);
		}
	}
}
