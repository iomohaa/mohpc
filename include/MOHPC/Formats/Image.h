#pragma once

#include "../Global.h"
#include "../Asset.h"
#include <memory>

namespace MOHPC
{
	enum class PixelFormat : uint8_t
	{
		IF_Unknown,
		IF_RGB,
		IF_RGBA
	};

	class Image : public Asset
	{
		CLASS_BODY(Image);

	public:
		MOHPC_EXPORTS uint8_t* GetData() const;
		MOHPC_EXPORTS uint32_t GetDataSize() const;
		MOHPC_EXPORTS PixelFormat GetPixelFormat() const;

		MOHPC_EXPORTS uint32_t GetWidth() const;
		MOHPC_EXPORTS uint32_t GetHeight() const;

	public:
		MOHPC_EXPORTS Image();
		MOHPC_EXPORTS ~Image();

	protected:
		virtual bool Load() override;

	private:
		void LoadJPEG(const char *name, void *buf, std::streamsize len);
		void LoadTGA(const char *name, void *buf, std::streamsize len);
		void LoadDDS(const char *name, void *buf, std::streamsize len);

	private:
		uint8_t *data;
		uint32_t dataSize;
		uint32_t width;
		uint32_t height;
		PixelFormat pixelFormat;
	};
	using ImagePtr = std::shared_ptr<Image>;
}
