#pragma once

#ifdef __linux__
#define stricmp strcasecmp
#define strnicmp strncasecmp
#define qsort2 qsort_r
#else
#define qsort2 qsort_s
#endif

#ifdef _MSC_VER
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif
