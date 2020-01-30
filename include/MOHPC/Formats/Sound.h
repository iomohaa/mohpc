#pragma once

#include "../Global.h"
#include "../Asset.h"
#include <memory>

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
		virtual bool Load() override;

	private:
		bool DecodeLAME(void *buf, std::streamsize len);
	};
	using SoundPtr = std::shared_ptr<Sound>;
}
