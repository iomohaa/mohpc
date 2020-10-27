#pragma once

#ifdef _WIN32
#include "win32/win_sockets.h"
#else
#include "unix/unix_sockets.h"
#endif

namespace MOHPC
{
	bool initSocket();
	void cleanupSocket();
	void closeSocket(socket_t socket);
	void ioctlSocket(int fd, unsigned long request, unsigned long* argp);
}
