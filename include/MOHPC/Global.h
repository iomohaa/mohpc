#pragma once

#undef MOHPC_EXPORTS

#ifdef MOHPC_DLL
	#ifdef __GNUC__
		#define MOHPC_EXPORTS __attribute__ ((visibility ("default")))
	#else
		#define MOHPC_EXPORTS __declspec(dllexport)
	#endif
#else
	#ifdef __GNUC__
		#define MOHPC_EXPORTS
	#else
		#define MOHPC_EXPORTS __declspec(dllimport)
	#endif
#endif
