#pragma once

#include "../UtilityGlobal.h"
#include "../../Common/str.h"

#include <cstdint>
#include <cstddef>

namespace MOHPC
{
	using StandardEscapeRule = class QuoteEscapeRule;

	namespace Escape_Private
	{
		template<typename T, typename EscapeRule = StandardEscapeRule>
		size_t EscapeStringLen(const T& value, EscapeRule escapeRule)
		{
			size_t len = 0;
			const char* p = static_cast<const char*>(value);
			while (*p)
			{
				if (escapeRule.shouldEscape(*p))
				{
					// also include the escape character
					++len;
				}

				++p;
				++len;
			}

			return len;
		}
	}

	template<typename T, typename EscapeRule = StandardEscapeRule>
	str EscapeString(const T& stringValue, EscapeRule escapeRule = EscapeRule())
	{
		// calculate the resulting string length to be able to allocate only once
		const size_t len = Escape_Private::EscapeStringLen(stringValue, escapeRule);

		str newString;
		newString.reserve(len);

		for (size_t i = 0; i < len; ++i)
		{
			const char character = stringValue[i];
			if (escapeRule.shouldEscape(character))
			{
				// escape the specified character
				newString += '\\';
			}

			newString += character;
		}

		return newString;
	}

	class QuoteEscapeRule
	{
	public:
		bool shouldEscape(char c)
		{
			// only escape quote character
			return c == '\"';
		}
	};
}