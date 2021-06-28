#include "Global.h"

namespace MOHPC
{
	// The following is automatically set and must not be modified
	extern MOHPC_EXPORTS const unsigned long VERSION_MAJOR;
	extern MOHPC_EXPORTS const unsigned long VERSION_MINOR;
	extern MOHPC_EXPORTS const unsigned long VERSION_PATCH;
	extern MOHPC_EXPORTS const unsigned long VERSION_BUILD;

	extern MOHPC_EXPORTS const char VERSION_STRING[];
	extern MOHPC_EXPORTS const char VERSION_SHORT_STRING[];
	extern MOHPC_EXPORTS const char VERSION_DATE[];

	// The following is manually set
	// This should be modified only for adding a new architecture
	extern MOHPC_EXPORTS const char VERSION_ARCHITECTURE[];
}
