#pragma once

#include "../Global.h"
#include "../Utility/SharedPtr.h"
#include "../Utility/WeakPtr.h"
#include <utility>

namespace MOHPC
{
	class AssetManager;

#define MOHPC_OBJECT_DECLARATION_BASE(c, exports) \
	public: \
	template<typename...Args> static SharedPtr<c> create(Args&&...args) { return makePtr(new (allocate()) c(std::forward<Args>(args)...)); } \
	private: \
	exports static SharedPtr<c> makePtr(c* ThisPtr); \
	exports static void destroy(c* instance); \
	exports static void* allocate();

#define MOHPC_OBJECT_DECLARATION(c) MOHPC_OBJECT_DECLARATION_BASE(c, MOHPC_EXPORTS)

#define MOHPC_OBJECT_DEFINITION(c) \
	MOHPC::SharedPtr<c> c::makePtr(c* ThisPtr) { return MOHPC::SharedPtr<c>(ThisPtr, &c::destroy); } \
	void c::destroy(c* instance) \
	{ \
		/** Extract type from namespace */ \
		using Type = c; \
		/** Call destructor */ \
		instance->~Type(); \
		/** Free up memory */ \
		delete[] reinterpret_cast<unsigned char*>(instance); \
	} \
	void* c::allocate() { return new unsigned char[sizeof(c)]; }
}
