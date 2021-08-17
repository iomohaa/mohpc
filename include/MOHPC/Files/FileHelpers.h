#pragma once

#include "../Common/str.h"
#include "FileDefs.h"

#include <filesystem>

namespace MOHPC
{
	namespace FileHelpers
	{
		template<typename T> static constexpr T extChar;
		template<> static constexpr char* extChar<char> = ".";
		template<> static constexpr wchar_t* extChar<wchar_t> = L".";
		template<> static constexpr char16_t* extChar<char16_t> = u".";
		template<> static constexpr char32_t* extChar<char32_t> = U".";

		template<typename T>
		static const T* getExtension(const T* val)
		{
			if (!val || !*val)
			{
				// empty string
				return strHelpers::emptyChar<T>;
			}

			const T* start = val;
			const T* end = val;
			while (*end) ++end;

			const T* p = end;
			while (p != start)
			{
				if (*p == *extChar<T>) break;
				--p;
			}

			if (*p == *extChar<T>) ++p;

			return p;
		}

		static fs::path getExtension(const fs::path& val)
		{
			using namespace fs;
			using charT = path::value_type;

			const charT* p = getExtension(val.c_str());

			return p;
		}

		template<typename TargetT, typename SourceT>
		static TargetT getExtension(const SourceT* val)
		{
			using namespace fs;
			const SourceT* p = getExtension(val);

			return fs::path(p).generic_string<TargetT::value_type>();
		}

		template<typename T>
		static fs::path SetFileExtension(const fs::path& Filename, const T* NewExtension)
		{
			using namespace fs;
			using charT = path::value_type;

			const charT* p = getExtension(Filename.c_str());

			const size_t newlen = p - Filename.c_str();
			path newfile = path::string_type().assign(Filename, newlen);
			newfile.append(extChar<charT>);
			newfile.append(NewExtension);

			return newfile;
		}

		template<typename T>
		static fs::path GetDefaultFileExtension(const fs::path& Filename, const T* DefaultExtension)
		{
			using namespace fs;
			using charT = path::value_type;

			const charT* p = getExtension(Filename.c_str());
			if (p) {
				return Filename;
			}

			path newfile = Filename;
			newfile.append(extChar<charT>);
			newfile.append(DefaultExtension);

			return newfile;
		}

		template<typename T>
		static const T* removeRootDir(const T* path)
		{
			if (*path == '/' || *path == '\\') ++path;
			return path;
		}

		static fstring_t CanonicalFilename(const fs::path& Filename)
		{
			using namespace fs;

			fstring_t newString;
			newString.reserve(Filename.native().length() + 1);

			for (const fchar_t* p = Filename.c_str(); *p; p++)
			{
				newString += *p;
				while (p[0] == '/' && p[1] == '/')
				{
					p++;
				}
			}

			return newString;
		}

		template<typename T>
		static std::basic_string<T> CanonicalFilename(const T* Filename)
		{
			using namespace fs;

			std::basic_string<T> newString;
			newString.reserve(strHelpers::len(Filename) + 1);

			for (const T* p = Filename; *p; p++)
			{
				newString += *p;
				while (p[0] == '/' && p[1] == '/')
				{
					p++;
				}
			}

			return newString;
		}
	}
}
