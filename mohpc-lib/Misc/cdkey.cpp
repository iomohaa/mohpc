#include <Shared.h>
#include <MOHPC/Misc/cdkey.h>
#include <MOHPC/Misc/crc32.h>
#include <string.h>
#include <algorithm>
#include <stdio.h>

namespace MOHPC
{
namespace CDKey
{

static char pkMap[] = { 2, 4, 5, 7, 12, 1, 3 };
static char hashMap[] = "64382957JKLMNPQRSTUVWXYZABCDEFGH";
static char crchashMap[] = "23456789ABCDEFGHJKLMNPQRSTUVWXYZ";

void UnmapCDKey(char* out, const char* key)
{
	strcpy(out, key);
	std::swap(out[pkMap[0]], out[13]);
	std::swap(out[pkMap[1]], out[14]);
	std::swap(out[pkMap[2]], out[15]);
	std::swap(out[pkMap[3]], out[16]);
	std::swap(out[pkMap[4]], out[17]);
	std::swap(out[pkMap[5]], out[18]);
	std::swap(out[pkMap[6]], out[19]);
	out[20] = 0;
}

int GetKeyHash(const char *key, int32_t gameProduct)
{
	char k1[29];
	char k2[21];
	char buf[8];

	strcpy(k1, "00000");
	strcat(k1, key);
	strcat(k1, "00000");
	strcpy(k2, key);
	strcat(k2, "00");

	int8_t const1 = (gameProduct >> 8) & 0xFF;
	int8_t const2 = gameProduct & 0xFF;

	int32_t mod1 = 0;
	{
		const size_t klen = strlen(k1);
		for (size_t i = 0; i < klen; i += 7)
		{
			strncpy(buf, &k1[i], 7);
			buf[7] = 0;
			mod1 = (mod1 + atoi(buf)) % const1;
		}
	}

	int32_t mod2 = 0;
	{
		const size_t klen = strlen(k2);
		for (size_t i = 0; i < klen; i += 5)
		{
			strncpy(buf, &k2[i], 5);
			buf[5] = 0;
			mod2 = (mod2 + atoi(buf)) % const2;
		}
	}

	return mod1 * 100 + mod2;
}

int GetKeyHash2(const char* keyPart, int length)
{
	int hash1 = 1;
	int hash2 = 0;

	for (size_t i = 0; i < length; i++)
	{
		hash1 = (hash1 + keyPart[i]) % 0xFFF1;
		hash2 = (hash2 + hash1) % 0xFFF1;
	}

	return hash1 + (hash2 << 16);
}

void GetHashBytes(char* out, int xorHash)
{
	out[6] = xorHash & 0x1F;
	out[5] = (xorHash >> 5) & 0x1F;
	out[4] = (xorHash >> 10) & 0x1F;
	out[3] = (xorHash >> 15) & 0x1F;
	out[2] = (xorHash >> 20) & 0x1F;
	out[1] = (xorHash >> 25) & 0x1F;
	out[0] = (xorHash >> 30) & 0x1F;

	out[0] = 4 * (out[6] & 7) | *out;

	out[0] = hashMap[out[0]];
	out[1] = hashMap[out[1]];
	out[2] = hashMap[out[2]];
	out[3] = hashMap[out[3]];
	out[4] = hashMap[out[4]];
	out[5] = hashMap[out[5]];
	out[6] = hashMap[out[6]];
	out[7] = 0;
}

void GetCrcHashBytes(char* out, int crchash)
{
	out[6] = (8 * (crchash & 3)) | (crchash >> 27) & 7;
	out[5] = (crchash >> 2) & 0x1F;
	out[4] = (crchash >> 7) & 0x1F;
	out[3] = (crchash >> 12) & 0x1F;
	out[2] = (crchash >> 17) & 0x1F;
	out[1] = (crchash >> 22) & 0x1F;
	out[0] = (crchash >> 27) & 0x1F;

	out[0] = crchashMap[out[0]];
	out[1] = crchashMap[out[1]];
	out[2] = crchashMap[out[2]];
	out[3] = crchashMap[out[3]];
	out[4] = crchashMap[out[4]];
	out[5] = crchashMap[out[5]];
	out[6] = crchashMap[out[6]];
	out[7] = 0;
}

void DecryptKey(char* out, const char* keyPart, int constval)
{
	int hash = GetKeyHash2(keyPart, 13);
	int xorHash = hash ^ constval;

	char decipheredKey[8];
	GetHashBytes(decipheredKey, xorHash);

	char tmpKey[32];
	strcpy(tmpKey, keyPart);
	strcat(tmpKey, decipheredKey);

	int crchash = crc32_hash(tmpKey, 20, constval);
	GetCrcHashBytes(decipheredKey, crchash);

	strcpy(tmpKey, keyPart);
	strcat(tmpKey, decipheredKey);
	UnmapCDKey(out, tmpKey);
}

void GenerateCDKey2(char* out, int32_t gameProduct)
{
	for (int i = 0; i < 13; i++)
	{
		int kt = rand() % 2;
		out[i] = kt == 0 ? ('0' + (rand() % 10)) : ('A' + (rand() % 26));
	}
	
	out[13] = 0;

	char tmpBuf[14];
	strncpy(tmpBuf, out, 14);
	tmpBuf[13] = 0;

	DecryptKey(out, tmpBuf, gameProduct);
}

void GenerateCDKey(char* out, int32_t gameProduct)
{
	for (int i = 0; i < 18; i++)
	{
		out[i] = '0' + (rand() % 10);
	}

	out[18] = 0;

	int hash = GetKeyHash(out, gameProduct);
	//itoa(hash, out + 18, 10);
	sprintf(out + 18, "%0.4d", hash);
}

void GenerateCDKeyFromBase2(char* out, int32_t gameProduct, const char* base)
{
	strcpy(out, base);

	char tmpBuf[14];
	strncpy(tmpBuf, out, 14);
	tmpBuf[13] = 0;

	DecryptKey(out, tmpBuf, gameProduct);
}

void GenerateCDKeyFromBase(char* out, int32_t gameProduct, const char* base)
{
	strcpy(out, base);

	int hash = GetKeyHash(out, gameProduct);
	//itoa(hash, out + 18, 10);
	sprintf(out + 18, "%0.4d", hash);
}

bool VerifyCDKey2(const char* key, int32_t gameProduct)
{
	char keyBuf[21];

	UnmapCDKey(keyBuf, key);
	keyBuf[13] = 0;

	char decryptedKey[21];
	DecryptKey(decryptedKey, keyBuf, gameProduct);

	return !strcmp(key, decryptedKey);
}

bool VerifyCDKey(const char* key, int32_t gameProduct)
{
	char keyBuf[19];

	strncpy(keyBuf, key, 18);
	keyBuf[18] = 0;
	int hash = GetKeyHash(keyBuf, gameProduct);

	int hash2 = atoi(key + 18);

	return hash == hash2 && hash && hash2;
}

}
};
