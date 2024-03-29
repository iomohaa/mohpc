#pragma once

#include <cstdint>
#include <string>
#include <algorithm>

namespace MOHPC
{
	using str = std::string;

	template<typename Archive, typename CharT>
	Archive& operator<<(Archive& ar, std::basic_string<CharT>& value)
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
	Archive& operator>>(Archive& ar, std::basic_string<CharT>& value)
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

	namespace strHelpers
	{
		template<typename T>
		static constexpr T emptyChar[] = { 0 };

		template<typename T>
		size_t len(const T* a)
		{
			size_t l = 0;
			while (*a)
			{
				++l;
				++a;
			}

			return l;
		}

		template<typename T>
		size_t cmp(const T* s1, const T* s2)
		{
			while (*s1 || *s2)
			{
				if (*s1 < *s2) return -1;
				if (*s1 > *s2) return 1;
				++s1; ++s2;
			}
			return 0;
		}

		template<typename T>
		size_t cmpn(const T* s1, const T* s2, size_t n)
		{
			while (n != 0)
			{
				if (*s1 < *s2) return -1;
				if (*s1 > *s2) return 1;
				++s1; ++s2;
				--n;
			}
			return 0;
		}

		template<typename T>
		int icmp(const T* s1, const T* s2)
		{
			while (*s1 || *s2)
			{
				const char c1 = std::tolower(*s1), c2 = std::tolower(*s2);
				if (c1 < c2) return -1;
				if (c1 > c2) return 1;
				++s1; ++s2;
			}
			return 0;
		}

		template<typename T>
		int icmpn(const T* s1, const T* s2, size_t n)
		{
			while (n != 0)
			{
				const char c1 = std::tolower(*s1), c2 = std::tolower(*s2);
				if (c1 < c2) return -1;
				if (c1 > c2) return 1;
				++s1; ++s2;
				--n;
			}
			return 0;
		}

		template<typename T>
		const T* findchar(const T* s, T c)
		{
			do {
				if (*s == c) return s;
			} while (*s++);

			return nullptr;
		}

		template<typename T>
		const T* findcharn(const T* s, T c, size_t n)
		{
			for (size_t i = 0; i < n && *s; ++i, ++s)
			{
				if (*s == c) {
					return s;
				}
			}
			return 0;
		}

		template<typename T>
		const T* find(const T* s1, const T* s2)
		{
			const size_t len = strHelpers::len(s2);
			for (const T* p = s1; (p = findchar(p, *s2)); ++p)
			{
				if (cmpn(p, s2, len) == 0) {
					return p;
				}
			}

			return 0;
		}

		template<typename T>
		const T* ifindchar(const T* s, T c)
		{
			do {
				if (std::tolower(*s) == std::tolower(c)) {
					return s;
				}
			} while (*s++);

			return 0;
		}

		template<typename T>
		const T* ifindcharn(const T* s, T c, size_t n)
		{
			for (size_t i = 0; i < n && *s; ++i, ++s)
			{
				if (std::tolower(*s) == std::tolower(c)) {
					return s;
				}
			}
			return 0;
		}

		template<typename T>
		const T* ifind(const T* s1, const T* s2)
		{
			const size_t l = len(s2);
			for (const T* p = s1; (p = ifindchar(p, *s2)); ++p)
			{
				if (icmpn(p, s2, l) == 0) {
					return p;
				}
			}

			return 0;
		}

		template<typename T>
		const T* ifindn(const T* s1, const T* s2, size_t n)
		{
			const size_t l = len(s2);
			for (const T* p = s1; (p = ifindchar(p, *s2)) && size_t(p - s1) < n; ++p)
			{
				if (icmpn(p, s2, l) == 0) {
					return p;
				}
			}

			return 0;
		}

		template<typename T>
		void tolower(T start, T end)
		{
			std::transform(start, end, start, [](typename T::value_type c) { return std::tolower(c); });
		}

		template<typename T>
		void tolower(typename std::basic_string<T>::iterator start, typename std::basic_string<T>::iterator end)
		{
			std::transform(start, end, start, [](typename T::value_type c) { return std::tolower(c); });
		}

		template<typename T>
		void toupper(T start, T end)
		{
			std::transform(start, end, start, [](typename T::value_type c) { return std::toupper(c); });
		}
	}
}
