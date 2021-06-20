#include "../generic_sockets.h"

using namespace MOHPC;

bool MOHPC::initSocket()
{
	WSADATA wsaData;

	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	return iResult == NO_ERROR;
}
void MOHPC::cleanupSocket()
{
	WSACleanup();
}

void MOHPC::closeSocket(socket_t socket)
{
	closesocket(socket);
}

void MOHPC::ioctlSocket(uintptr_t fd, unsigned long request, unsigned long* argp)
{
	ioctlsocket(fd, request, argp);
}