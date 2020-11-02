#pragma once

namespace MOHPC
{
	template<typename T>
	void copyToAlign(T& dest, const void* source)
	{
		memcpy(&dest, source, sizeof(T));
	}

	template<typename T>
	void copyToAlign(T& dest, const void* source, size_t numElements)
	{
		memcpy(&dest, source, sizeof(T) * numElements);
	}
}
