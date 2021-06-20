#pragma once

#include <morfuse/Common/str.h>

namespace MOHPC
{
	using str = mfuse::str;

	template<typename Archive, typename CharT>
	Archive& operator<<(Archive& ar, mfuse::base_str<CharT>& value)
	{
		if (value.length() > 0)
		{
			ar << (uint32_t)value.length();

			for (size_t i = 0; i < value.length(); ++i)
			{
				ar << value.c_str()[i];
			}
		}
		else {
			ar << (uint32_t)0;
		}

		return ar;
	}

	template<typename Archive, typename CharT>
	Archive& operator>>(Archive& ar, mfuse::base_str<CharT>& value)
	{
		uint32_t len = 0;
		ar >> len;

		if (len)
		{
			value.resize(len);
			for (size_t i = 0; i < len; ++i)
			{
				ar >> value[i];
			}
		}

		return ar;
	}
}
