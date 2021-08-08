#pragma once

#include <MOHPC/Assets/Managers/AssetManager.h>
#include <MOHPC/Files/Managers/FileManager.h>
#include <MOHPC/Common/Log.h>

const char* GetGamePathFromCommandLine(int argc, const char* argv[]);
MOHPC::AssetManagerPtr AssetLoad(const char* path);
void InitCommon();
