#include <cstring>
#include <fstream>
#include <MOHPC/Misc/SHA256.h>

using namespace MOHPC;

#define ROTLEFT(a,b) (((a) << (b)) | ((a) >> (32-(b))))
#define ROTRIGHT(a,b) (((a) >> (b)) | ((a) << (32-(b))))

#define CH(x,y,z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x,y,z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x) (ROTRIGHT(x,2) ^ ROTRIGHT(x,13) ^ ROTRIGHT(x,22))
#define EP1(x) (ROTRIGHT(x,6) ^ ROTRIGHT(x,11) ^ ROTRIGHT(x,25))
#define SIG0(x) (ROTRIGHT(x,7) ^ ROTRIGHT(x,18) ^ ((x) >> 3))
#define SIG1(x) (ROTRIGHT(x,17) ^ ROTRIGHT(x,19) ^ ((x) >> 10))

static const unsigned int k[64] = {
	0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
	0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
	0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
	0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
	0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
	0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
	0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
	0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

void SHA256::Init()
{
	m_datalen = 0;
	m_bitlen = 0;
	m_state[0] = 0x6a09e667;
	m_state[1] = 0xbb67ae85;
	m_state[2] = 0x3c6ef372;
	m_state[3] = 0xa54ff53a;
	m_state[4] = 0x510e527f;
	m_state[5] = 0x9b05688c;
	m_state[6] = 0x1f83d9ab;
	m_state[7] = 0x5be0cd19;
}

void SHA256::Update(const void* data, size_t len)
{
	unsigned int i;
	const unsigned char* l_data = (const unsigned char*)data;

	for (i = 0; i < len; ++i) {
		m_data[m_datalen] = l_data[i];
		m_datalen++;
		if (m_datalen == 64) {
			Transform(m_data);
			m_bitlen += 512;
			m_datalen = 0;
		}
	}
}

std::string SHA256::Final()
{
	unsigned int i;

	i = m_datalen;

	// Pad whatever data is left in the buffer.
	if (m_datalen < 56) {
		m_data[i++] = 0x80;
		while (i < 56)
			m_data[i++] = 0x00;
	}
	else {
		m_data[i++] = 0x80;
		while (i < 64)
			m_data[i++] = 0x00;
		Transform(m_data);
		memset(m_data, 0, 56);
	}

	// Append to the padding the total message's length in bits and transform.
	m_bitlen += m_datalen * 8;
	m_data[63] = (unsigned char)m_bitlen;
	m_data[62] = (unsigned char)(m_bitlen >> 8);
	m_data[61] = (unsigned char)(m_bitlen >> 16);
	m_data[60] = (unsigned char)(m_bitlen >> 24);
	m_data[59] = (unsigned char)(m_bitlen >> 32);
	m_data[58] = (unsigned char)(m_bitlen >> 40);
	m_data[57] = (unsigned char)(m_bitlen >> 48);
	m_data[56] = (unsigned char)(m_bitlen >> 56);
	Transform(m_data);

	unsigned char hash[32];

	// Since this implementation uses little endian byte ordering and SHA uses big endian,
	// reverse all the bytes when copying the final state to the output hash.
	for (i = 0; i < 4; ++i) {
		hash[i] = (m_state[0] >> (24 - i * 8)) & 0x000000ff;
		hash[i + 4] = (m_state[1] >> (24 - i * 8)) & 0x000000ff;
		hash[i + 8] = (m_state[2] >> (24 - i * 8)) & 0x000000ff;
		hash[i + 12] = (m_state[3] >> (24 - i * 8)) & 0x000000ff;
		hash[i + 16] = (m_state[4] >> (24 - i * 8)) & 0x000000ff;
		hash[i + 20] = (m_state[5] >> (24 - i * 8)) & 0x000000ff;
		hash[i + 24] = (m_state[6] >> (24 - i * 8)) & 0x000000ff;
		hash[i + 28] = (m_state[7] >> (24 - i * 8)) & 0x000000ff;
	}

	char buf[2 * 32 + 1];
	buf[2 * 32] = 0;
	for (int i = 0; i < 32; i++)
		sprintf(buf + i * 2, "%02x", hash[i]);

	return std::string(buf);
}

void SHA256::Transform(const void* data)
{
	unsigned int a, b, c, d, e, f, g, h, i, j, t1, t2, m[64];
	const unsigned char* l_data = (const unsigned char*)data;

	for (i = 0, j = 0; i < 16; ++i, j += 4)
		m[i] = (l_data[j] << 24) | (l_data[j + 1] << 16) | (l_data[j + 2] << 8) | (l_data[j + 3]);
	for (; i < 64; ++i)
		m[i] = SIG1(m[i - 2]) + m[i - 7] + SIG0(m[i - 15]) + m[i - 16];

	a = m_state[0];
	b = m_state[1];
	c = m_state[2];
	d = m_state[3];
	e = m_state[4];
	f = m_state[5];
	g = m_state[6];
	h = m_state[7];

	for (i = 0; i < 64; ++i) {
		t1 = h + EP1(e) + CH(e, f, g) + k[i] + m[i];
		t2 = EP0(a) + MAJ(a, b, c);
		h = g;
		g = f;
		f = e;
		e = d + t1;
		d = c;
		c = b;
		b = a;
		a = t1 + t2;
	}

	m_state[0] += a;
	m_state[1] += b;
	m_state[2] += c;
	m_state[3] += d;
	m_state[4] += e;
	m_state[5] += f;
	m_state[6] += g;
	m_state[7] += h;
}
