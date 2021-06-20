#pragma once

#include "../Asset.h"
#include "../../Utility/SharedPtr.h"

namespace MOHPC
{
	class Sound : public Asset
	{
		MOHPC_ASSET_OBJECT_DECLARATION(Sound);

	private:
		uint8_t * data;
		size_t dataLen;

	public:
		MOHPC_ASSETS_EXPORTS uint8_t * GetData() const;
		MOHPC_ASSETS_EXPORTS size_t GetDataLength() const;

	public:
		MOHPC_ASSETS_EXPORTS Sound();
		MOHPC_ASSETS_EXPORTS ~Sound();

	protected:
		void Load() override;

	private:
		void DecodeLAME(void *buf, uint64_t len);
	};
	using SoundPtr = SharedPtr<Sound>;

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
