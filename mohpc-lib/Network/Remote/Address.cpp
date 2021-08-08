#include <MOHPC/Network/Remote/Address.h>

using namespace MOHPC;
using namespace MOHPC::Network;


NetAddr::NetAddr()
	: port(0)
{}

void NetAddr::setPort(uint16_t value)
{
	port = value;
}

uint16_t NetAddr::getPort() const
{
	return port;
}

bool NetAddr::operator==(const NetAddr& other) const
{
	const size_t sz = getAddrSize();
	if (sz != other.getAddrSize()) {
		return false;
	}

	const uint8_t* buf = getAddress();
	return !memcmp(buf, other.getAddress(), sz);
}

bool NetAddr::operator!=(const NetAddr& other) const
{
	return !(*this == other);
}

MOHPC_OBJECT_DEFINITION(NetAddr4);

uint32_t numDigits(uint32_t number)
{
	return (uint32_t)floorf(log10f((float)number) + 1);
}

NetAddr4::NetAddr4()
	: ip{ 0 }
{
}

void NetAddr4::setIp(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{
	ip[0] = a;
	ip[1] = b;
	ip[2] = c;
	ip[3] = d;
}

size_t NetAddr4::getAddrSize() const
{
	return sizeof(ip);
}

const uint8_t* NetAddr4::getAddress() const
{
	return ip;
}

str NetAddr4::asString() const
{
	size_t ipNums = 0;
	for (size_t i = 0; i < sizeof(ip); ++i) {
		ipNums += numDigits(ip[i]) + 1;
	}

	str adrBuf;
	adrBuf.reserve(ipNums + 1 + numDigits(port));
	for (size_t i = 0; i < sizeof(ip); ++i)
	{
		if (i != 0) adrBuf += '.';
		adrBuf += ip[i];
	}

	adrBuf += ":" + std::to_string(port);

	return adrBuf;
}

MOHPC_OBJECT_DEFINITION(NetAddr6);

NetAddr6::NetAddr6()
	: ip{ 0 }
{
}

void NetAddr6::setIp(uint16_t a, uint16_t b, uint16_t c, uint16_t d, uint16_t e, uint16_t f, uint16_t g, uint16_t h)
{
	ip[0] = a & 0xFF; ip[1] = (a >> 8) & 0xFF;
	ip[2] = b & 0xFF; ip[3] = (b >> 8) & 0xFF;
	ip[4] = c & 0xFF; ip[5] = (c >> 8) & 0xFF;
	ip[6] = d & 0xFF; ip[7] = (d >> 8) & 0xFF;
	ip[8] = e & 0xFF; ip[9] = (e >> 8) & 0xFF;
	ip[10] = f & 0xFF; ip[11] = (f >> 8) & 0xFF;
	ip[12] = g & 0xFF; ip[13] = (g >> 8) & 0xFF;
	ip[14] = h & 0xFF; ip[15] = (h >> 8) & 0xFF;
}

size_t NetAddr6::getAddrSize() const
{
	return sizeof(ip);
}

const uint8_t* NetAddr6::getAddress() const
{
	return ip;
}

str NetAddr6::asString() const
{
	size_t ipNums = 0;
	for (size_t i = 0; i < sizeof(ip); ++i) {
		ipNums += numDigits(ip[i]) + 1;
	}

	str adrBuf;
	adrBuf.reserve(ipNums + 1 + numDigits(port));
	for (size_t i = 0; i < sizeof(ip); ++i)
	{
		if (i != 0) adrBuf += "::";
		adrBuf += ip[i];
	}

	return adrBuf;
}
