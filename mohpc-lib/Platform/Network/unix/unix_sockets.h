#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

namespace MOHPC
{
	using socket_t = int;
	static constexpr socket_t badSocket = 0;
}
