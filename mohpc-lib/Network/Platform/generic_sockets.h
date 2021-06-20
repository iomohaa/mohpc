#pragma once

#ifdef _WIN32
#include "win32/win_sockets.h"
#else
#include "unix/unix_sockets.h"
#endif

#include <cstdint>
#include <cstddef>

namespace MOHPC
{
	bool initSocket();
	void cleanupSocket();
	void closeSocket(socket_t socket);
	void ioctlSocket(uintptr_t fd, unsigned long request, unsigned long* argp);
}
