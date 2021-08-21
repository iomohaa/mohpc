#pragma once

#include <MOHPC/Assets/Managers/AssetManager.h>
#include <MOHPC/Files/Managers/IFileManager.h>
#include <MOHPC/Files/FileDefs.h>
#include <MOHPC/Common/Log.h>

const MOHPC::fs::path& GetGamePathFromCommandLine();
MOHPC::AssetManagerPtr AssetLoad(const MOHPC::fs::path& path);
void InitCommon(int argc, const char* argv[]);
