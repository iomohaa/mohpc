#pragma once

#include "AssetBase.h"
#include "../Common/Object.h"
#include "../Common/str.h"
#include "../Files/File.h"
#include "../Files/FileDefs.h"

namespace MOHPC
{
	class Asset
	{
	public:
		Asset(const fs::path& fileNameRef);
		virtual ~Asset();

		/** Returns the filename of the asset. */
		MOHPC_ASSETS_EXPORTS const fs::path& getFilename() const;

	private:
		fs::path fileName;
	};
	using AssetPtr = SharedPtr<Asset>;

	class AssetReader : public AssetObject
	{
	public:
		MOHPC_ASSETS_EXPORTS virtual AssetPtr read(const IFilePtr& file) = 0;
	};
	using AssetReaderPtr = SharedPtr<AssetReader>;

	class AssetWriter : public AssetObject
	{
	};
	using AssetWriterPtr = SharedPtr<AssetWriter>;
}
