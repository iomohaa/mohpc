#pragma once

#include <memory>

namespace MOHPC
{
	template<typename T>
	using UniquePtr = std::unique_ptr<T>;
}
