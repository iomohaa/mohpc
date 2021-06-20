#pragma once

#undef MOHPC_NET_EXPORTS

#if defined(MOHPC_NET_DLL)
	#ifdef __GNUC__
		#define MOHPC_NET_EXPORTS __attribute__ ((visibility ("default")))
	#else
		#define MOHPC_NET_EXPORTS __declspec(dllexport)
	#endif
	#define MOHPC_NET_TEMPLATE
#else
	#ifdef __GNUC__
		#define MOHPC_NET_EXPORTS
	#else
		#define MOHPC_NET_EXPORTS __declspec(dllimport)
	#endif
	#define MOHPC_NET_TEMPLATE extern
#endif
