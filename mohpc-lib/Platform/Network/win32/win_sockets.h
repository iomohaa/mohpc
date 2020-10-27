#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include "win_sockets.h"

#pragma comment(lib, "Ws2_32.lib")

namespace MOHPC
{
	using socket_t = SOCKET;
	static constexpr socket_t badSocket = (socket_t)INVALID_SOCKET;
}