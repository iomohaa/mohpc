#include <MOHPC/Network/Serializable/NetField.h>

using namespace MOHPC;
using namespace MOHPC::Network;

const char* netField_t::getName() const
{
	return name;
}

uint16_t netField_t::getOffset() const
{
	return offset;
}

uint8_t netField_t::getSize() const
{
	return size;
}

uint8_t netField_t::getBits() const
{
	return bits;
}

uint8_t netField_t::getType() const
{
	return type;
}
