#pragma once

#undef MOHPC_UTILITY_EXPORTS

#if defined(MOHPC_UTILITY_DLL)
	#ifdef __GNUC__
		#define MOHPC_UTILITY_EXPORTS __attribute__ ((visibility ("default")))
	#else
		#define MOHPC_UTILITY_EXPORTS __declspec(dllexport)
	#endif
	#define MOHPC_UTILITY_TEMPLATE
#else
	#ifdef __GNUC__
		#define MOHPC_UTILITY_EXPORTS
	#else
		#define MOHPC_UTILITY_EXPORTS __declspec(dllimport)
	#endif
	#define MOHPC_UTILITY_TEMPLATE extern
#endif
