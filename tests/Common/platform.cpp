#include "platform.h"

#ifdef _WIN32
#include <conio.h>
#include <Windows.h>
#include <timeapi.h>
#else
#include <curses.h>
#include <unistd.h>
#include <termios.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#endif

#ifndef _WIN32
static const unsigned int STDIN = STDIN_FILENO;

void initConsole()
{
}

void sleepTime(size_t ms)
{
	usleep(ms * 1000);
}

bool isKeyboardHit()
{
/*
	timeval timeout;
	fd_set rdset;

	FD_ZERO(&rdset);
	FD_SET(STDIN, &rdset);
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	return select(STDIN + 1, &rdset, NULL, NULL, &timeout);
*/
	int bytesWaiting = 0;
	ioctl(STDIN, FIONREAD, &bytesWaiting);
	return bytesWaiting;
}

int getConsoleChar()
{
	return getchar();
}

void setTimerResolution(uint64_t resolution)
{
}

void restoreTimerSolution()
{
}

#else
void initConsole()
{
}

void sleepTime(size_t ms)
{
	Sleep((int)ms);
}

bool isKeyboardHit()
{
	return kbhit();
}

int getConsoleChar()
{
	return _getch();
}

void setTimerResolution(uint64_t resolution)
{
	timeBeginPeriod((UINT)resolution);
}

void restoreTimerSolution(uint64_t resolution)
{
	timeEndPeriod((UINT)resolution);
}
#endif
