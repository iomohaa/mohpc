#pragma once

#undef MOHPC_ASSETS_EXPORTS

#if defined(MOHPC_ASSETS_DLL)
	#ifdef __GNUC__
		#define MOHPC_ASSETS_EXPORTS __attribute__ ((visibility ("default")))
	#else
		#define MOHPC_ASSETS_EXPORTS __declspec(dllexport)
	#endif
	#define MOHPC_ASSETS_TEMPLATE
#else
	#ifdef __GNUC__
		#define MOHPC_ASSETS_EXPORTS
	#else
		#define MOHPC_ASSETS_EXPORTS __declspec(dllimport)
	#endif
	#define MOHPC_ASSETS_TEMPLATE extern
#endif
