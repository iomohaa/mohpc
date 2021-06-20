#pragma once

#undef MOHPC_FILES_EXPORTS

#if defined(MOHPC_FILES_DLL)
	#ifdef __GNUC__
		#define MOHPC_FILES_EXPORTS __attribute__ ((visibility ("default")))
	#else
		#define MOHPC_FILES_EXPORTS __declspec(dllexport)
	#endif
	#define MOHPC_FILES_TEMPLATE
#else
	#ifdef __GNUC__
		#define MOHPC_FILES_EXPORTS
	#else
		#define MOHPC_FILES_EXPORTS __declspec(dllimport)
	#endif
	#define MOHPC_FILES_TEMPLATE extern
#endif
