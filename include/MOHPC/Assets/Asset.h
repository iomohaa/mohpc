#pragma once

#include "AssetBase.h"
#include "../Common/Object.h"
#include "../Common/str.h"
#include "../Files/File.h"
#include "../Files/FileDefs.h"

namespace MOHPC
{
	class Asset : public AssetObject
	{
	friend AssetManager;

	public:
		virtual ~Asset();

		/** Returns the filename of the asset. */
		MOHPC_ASSETS_EXPORTS const fs::path& GetFilename() const;

		/** Returns the 20-bit hash for this asset. */
		MOHPC_ASSETS_EXPORTS void HashGetHash(uint8_t* Destination) const;

	protected:
		MOHPC_ASSETS_EXPORTS Asset();

	private:
		// Called by the asset manager
		void Init(const fs::path& Filename);
		void HashFinalize();

	protected:
		void HashUpdate(const uint8_t* Data, uint64_t Length);
		void HashCopy(const Asset* A);

		virtual void Load() = 0;

	private:
		fs::path Filename;
		class Hasher* Hash;
	};

	class Asset2
	{
	public:
		Asset2(const fs::path& fileNameRef);
		virtual ~Asset2();

		/** Returns the filename of the asset. */
		MOHPC_ASSETS_EXPORTS const fs::path& getFilename() const;

	private:
		fs::path fileName;
	};
	using Asset2Ptr = SharedPtr<Asset2>;

	class AssetReader : public AssetObject
	{
	public:
		MOHPC_ASSETS_EXPORTS virtual Asset2Ptr read(const IFilePtr& file) = 0;
	};
	using AssetReaderPtr = SharedPtr<AssetReader>;

	class AssetWriter : public AssetObject
	{
	};
	using AssetWriterPtr = SharedPtr<AssetWriter>;
}
