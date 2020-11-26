#include "../generic_sockets.h"

using namespace MOHPC;

bool MOHPC::initSocket()
{
	return true;
}

void MOHPC::cleanupSocket()
{
}

void MOHPC::closeSocket(socket_t socket)
{
	close(socket);
}

void MOHPC::ioctlSocket(uintptr_t fd, unsigned long request, unsigned long* argp)
{
	ioctl(fd, request, argp);
}
