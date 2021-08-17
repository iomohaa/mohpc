#pragma once

#include "../Asset.h"
#include "../../Utility/SharedPtr.h"

namespace MOHPC
{
	class Sound : public Asset2
	{
		MOHPC_ASSET_OBJECT_DECLARATION(Sound);

	public:
		MOHPC_ASSETS_EXPORTS Sound(const fs::path& fileName, uint8_t* dataPtr, size_t size);
		MOHPC_ASSETS_EXPORTS ~Sound();

	public:
		MOHPC_ASSETS_EXPORTS uint8_t* GetData() const;
		MOHPC_ASSETS_EXPORTS size_t GetDataLength() const;

	private:
		uint8_t* data;
		size_t dataLen;
	};
	using SoundPtr = SharedPtr<Sound>;

	class SoundReader : public AssetReader
	{
		MOHPC_ASSET_OBJECT_DECLARATION(SoundReader);

	public:
		using AssetType = Sound;

	public:
		MOHPC_ASSETS_EXPORTS Asset2Ptr read(const IFilePtr& file) override;

	private:
		Asset2Ptr DecodeLAME(const fs::path& fileName, void* buf, uint64_t len);
	};

	namespace SoundError
	{
		class Base : public std::exception {};

		class BadOrUnsupportedSound : public Base
		{
		public:
			BadOrUnsupportedSound(const str& extension);

			MOHPC_ASSETS_EXPORTS const char* getExtension() const;

		public:
			const char* what() const noexcept override;

		private:
			str extension;
		};
	}
}
