#pragma once

#include "Utilities/SharedPtr.h"
#include <utility>

namespace MOHPC
{
#define MOHPC_OBJECT_DECLARATION(c) \
	public: \
	template<typename...Args> static SharedPtr<c> create(Args&&...args) { return makePtr(new (allocate()) c(std::forward<Args>(args)...)); } \
	private: \
	MOHPC_EXPORTS static SharedPtr<c> makePtr(c* ThisPtr); \
	MOHPC_EXPORTS static void destroy(c* instance); \
	MOHPC_EXPORTS static void* allocate(); \

#define MOHPC_OBJECT_DEFINITION(c) \
	SharedPtr<c> c::makePtr(c* ThisPtr) { return SharedPtr<c>(ThisPtr, &c::destroy); } \
	void c::destroy(c* instance) { instance->~c(); delete[] reinterpret_cast<unsigned char*>(instance); } \
	void* c::allocate() { return new unsigned char[sizeof(c)]; };
}
