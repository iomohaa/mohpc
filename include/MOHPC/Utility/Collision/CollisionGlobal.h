#pragma once

#undef MOHPC_COLLISION_EXPORTS

#if defined(MOHPC_COLLISION_DLL)
	#ifdef __GNUC__
		#define MOHPC_COLLISION_EXPORTS __attribute__ ((visibility ("default")))
	#else
		#define MOHPC_COLLISION_EXPORTS __declspec(dllexport)
	#endif
	#define MOHPC_COLLISION_TEMPLATE
#else
	#ifdef __GNUC__
		#define MOHPC_COLLISION_EXPORTS
	#else
		#define MOHPC_COLLISION_EXPORTS __declspec(dllimport)
	#endif
	#define MOHPC_COLLISION_TEMPLATE extern
#endif
