#pragma once

#include "FileDefs.h"
#include "FileHelpers.h"
#include <filesystem>

namespace MOHPC
{
	struct FileNameCompare
	{
		template<typename T>
		static bool CompareCharPath(T c1, T c2)
		{
			return tolower(c1) < tolower(c2);
		}

		template<typename T>
		static bool CompareCharPathEquality(T c1, T c2)
		{
			return tolower(c1) == tolower(c2);
		}

		template<typename T>
		static const T* StripBase(const T* path, const T* base)
		{
			const T* p1 = path;
			const T* p2 = base;

			while (*p1 && *p2)
			{
				if (tolower(*p1++) != tolower(*p2++))
				{
					return p1;
				}
			}

			return p1;
		}

		template<typename T>
		static size_t GetParentDir(const T& path)
		{
			using charT = T::value_type;

			const charT* str = path.c_str();
			for (const charT* p = str + path.native().length() - 1; p != str - 1; p--)
			{
				if (*p == '/')
				{
					return p - str + 1;
				}
			}

			return 0;
		}

		template<typename T>
		static const typename T::value_type* GetFilename(const T& path)
		{
			using charT = T::value_type;

			const charT* str = path.c_str();
			for (const charT* p = str + path.native().length() - 1; p != str - 1; p--)
			{
				if (*p == '/')
				{
					return p + 1;
				}
			}

			return str;
		}

		bool operator() (const fs::path& s1, const fs::path& s2) const
		{
			using charT = fs::path::value_type;

			const size_t parentdirl1 = GetParentDir(s1);
			const size_t parentdirl2 = GetParentDir(s2);
			charT* p1 = const_cast<charT*>(s1.c_str());
			charT* p2 = const_cast<charT*>(s2.c_str());
			charT* pend1 = p1 + parentdirl1;
			charT* pend2 = p2 + parentdirl2;
			const charT ch1 = *pend1;
			const charT ch2 = *pend2;
			//int comp = ls1.compare(0, parentdirl1, ls2.c_str(), parentdirl2);
			//bool comp = std::equal(p1 + 1, p1 + parentdirl1, p2 + 1, p2 + parentdirl2, &CompareCharPathEquality);

			// HACK HACK : Faster than std::equal combined with std::lexicographical_compare
			*pend1 = 0;
			*pend2 = 0;
			int comp = strHelpers::icmp(p1 + 1, p2 + 1);
			*pend1 = ch1;
			*pend2 = ch2;
			if (!comp)
			{
				const charT* fname1 = GetFilename(s1);
				const charT* fname2 = GetFilename(s2);

				if ((*fname1 == '.') ^ (*fname2 == '.'))
				{
					return *fname1 == '.';
				}
				else
				{
					return strHelpers::icmp(fname1, fname2) < 0;
				}
			}
			return comp < 0;
			//return std::lexicographical_compare(p1 + 1, p1 + parentdirl1, p2 + 1, p2 + parentdirl2, &CompareCharPath);
		}
	};

	struct FileNameMapCompare
	{
		template<typename T>
		bool operator() (const T& s1, const T& s2) const
		{
			return !strHelpers::icmp(FileHelpers::removeRootDir(s1.c_str()), FileHelpers::removeRootDir(s2.c_str()));
		}
	};

	struct FileNameHash
	{
		template<typename T>
		size_t operator()(const T& s1) const
		{
			size_t hash = 0;

			const T::value_type* p = s1.c_str();
			if (*p == '\\' || *p == '/') ++p;

			while (*p) {
				hash = tolower(*p++) + 31 * hash;
			}

			return hash;
		}
	};
}