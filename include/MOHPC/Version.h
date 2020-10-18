#include <stdint.h>

namespace MOHPC
{
	// The following is automatically set and must not be modified
	static constexpr unsigned long VERSION_MAJOR = 1;
	static constexpr unsigned long VERSION_MINOR = 1;
	static constexpr unsigned long VERSION_BUILD = 9107;

	static constexpr char VERSION_STRING[] = "1.1.9107";
	static constexpr char VERSION_SHORT_STRING[] = "1.1";
	static constexpr char VERSION_DATE[] = "Oct 18 2020";

	// The following is manually set
	// This should be modified only for adding a new architecture
	static constexpr char VERSION_ARCHITECTURE[] =
#ifdef _WIN32
		"win"
#elif defined (__linux__)
		"linux"
#elif defined(__unix__)
		"unix"
#else
		"unknown"
#endif

		"-"

#if defined(__x86_64__) || defined(_WIN64_) || defined(_M_X64)
		"x86_64"
#elif defined (__ia64__) || defined (_M_IA64)
		"ia64"
#elif defined(__arm__)
		"arm"
#elif defined(__aarch64__)
		"aarch64"
#elif defined(__i386)
		"x86"
#else
		"unknown"
#endif

		"-"

#if UINTPTR_MAX == 0xffffffffffffffffULL 
		"64"
#else
		"32"
#endif
		;
}
