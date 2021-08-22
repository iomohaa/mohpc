#include <MOHPC/Version.h>

#include <cstdint>

namespace MOHPC
{
	// The following is automatically set and must not be modified
	const unsigned long VERSION_MAJOR = 1;
	const unsigned long VERSION_MINOR = 3;
	const unsigned long VERSION_PATCH = 0;
	const unsigned long VERSION_BUILD = 23651;

	const char VERSION_STRING[] = "1.3.0.23651";
	const char VERSION_SHORT_STRING[] = "1.3.0";
	const char VERSION_DATE[] = "Aug 22 2021";

	// The following is manually set
	// This should be modified only for adding a new architecture
	const char VERSION_ARCHITECTURE[] =
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

#if defined(__x86_64__) || defined(_WIN64_) || defined(_M_X64) || defined(_M_AMD64)
		"x86_64"
#elif defined(__i386)
		"x86"
#elif defined (__ia64__) || defined (_M_IA64)
		"ia64"
#elif defined(__arm__) || defined(_M_ARM)
		"arm"
#elif defined(__aarch64__)
		"aarch64"
#elif defined(__mips__) || defined(__mips)
		"mips"
#elif defined(__powerpc__) || defined (__ppc__) 
		"powerpc"
#elif defined(__powerpc64__) || defined (__ppc64__)
		"ppc64"
#elif defined(__sparc__)
		"sparc"
#elif defined(__370__)
		"370"
#elif defined(__s390__)
		"s390x"
#elif defined(__s390x__)
		"s390x"
#elif defined(__hppa__)
		"hppa"
#elif defined(__alpha__) || defined (_M_ALPHA)
		"alpha"
#else
		"unknown"
#endif

		"-"

#if UINTPTR_MAX == 0xffffffffffffffffULL 
		"64"
#else
		"32"
#endif
		"-"
#if (!defined(__BYTE_ORDER__) || (__BYTE_ORDER__ != __ORDER_BIG_ENDIAN__)) \
	&& (!defined(__BYTE_ORDER) || (__BYTE_ORDER != __BIG_ENDIAN)) \
	&& (!defined(_BYTE_ORDER) || (_BYTE_ORDER != _BIG_ENDIAN)) \
	&& (!defined(BYTE_ORDER) || (BYTE_ORDER != BIG_ENDIAN))
		"le"
#else
		"be"
#endif
		;
}
