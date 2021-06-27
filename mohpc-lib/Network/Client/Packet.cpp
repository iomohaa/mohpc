#include <MOHPC/Network/Client/Packet.h>

using namespace MOHPC;
using namespace MOHPC::Network;

outPacket_t::outPacket_t()
	: p_cmdNumber(0)
	, p_serverTime(0)
	, p_currentTime(0)
{

}