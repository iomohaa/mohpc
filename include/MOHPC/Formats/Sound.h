#pragma once

#include "../Global.h"
#include "../Asset.h"
#include "../Utilities/SharedPtr.h"

namespace MOHPC
{
	class Sound : public Asset
	{
		CLASS_BODY(Sound);

	private:
		uint8_t * data;
		size_t dataLen;

	public:
		MOHPC_EXPORTS uint8_t * GetData() const;
		MOHPC_EXPORTS size_t GetDataLength() const;

	public:
		MOHPC_EXPORTS Sound();
		MOHPC_EXPORTS ~Sound();

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

			MOHPC_EXPORTS const char* getExtension() const;

		public:
			const char* what() const override;

		private:
			str extension;
		};
	}
}
