#pragma once

#include <filesystem>

namespace MOHPC
{
	namespace fs = std::filesystem;
	using fchar_t = fs::path::value_type;
	using fstring_t = fs::path::string_type;
}