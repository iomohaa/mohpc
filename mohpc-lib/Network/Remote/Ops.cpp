#include <MOHPC/Network/Remote/Ops.h>

using namespace MOHPC;
using namespace MOHPC::Network;

OpsErrors::BadCommandByteException::BadCommandByteException(uint8_t inCmdNum)
	: cmdNum(inCmdNum)
{}

uint8_t OpsErrors::BadCommandByteException::getLength() const
{
	return cmdNum;
}

str OpsErrors::BadCommandByteException::what() const
{
	return str((int)getLength());
}
