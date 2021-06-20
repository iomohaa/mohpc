#pragma once

#include <stdarg.h>

class ImageException : public std::exception
{
private:
	char buf[1000];

public:
	ImageException(char const* fmt, ...)
	{
		va_list ap;
		va_start(ap, fmt);
		vsnprintf(buf, sizeof(buf), fmt, ap);
		va_end(ap);
	}

	virtual char const* what() const noexcept
	{
		return buf;
	}
};
