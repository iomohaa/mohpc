#pragma once

#include "../Global.h"
#include "../Asset.h"
#include "../Utilities/SharedPtr.h"

namespace MOHPC
{
	enum class PixelFormat : uint8_t
	{
		Unknown,
		RGB,
		RGBA
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
		void Load() override;

	private:
		void LoadJPEG(const char *name, void *buf, uint64_t len);
		void LoadTGA(const char *name, void *buf, uint64_t len);
		void LoadDDS(const char *name, void *buf, uint64_t len);

	private:
		uint8_t *data;
		uint32_t dataSize;
		uint32_t width;
		uint32_t height;
		PixelFormat pixelFormat;
	};
	using ImagePtr = SharedPtr<Image>;

	namespace ImageError
	{
		class Base : public std::exception {};

		/**
		 * The image file has a bad or unsupported extension.
		 */
		class BadExtension : public Base
		{
		public:
			BadExtension(const str& extension);

			MOHPC_EXPORTS const char* getExtension() const;

		public:
			const char* what() const override;

		private:
			str extension;
		};

		/**
		 * Header too short.
		 */
		class HeaderTooShort : public Base
		{
		public:
			HeaderTooShort(uint64_t len);

			MOHPC_EXPORTS uint64_t getLength() const;

		public:
			const char* what() const override;

		private:
			uint64_t len;
		};

		/**
		 * The file is truncated.
		 */
		class FileTruncated : public Base
		{
		public:
			FileTruncated(uint64_t expectedSize, uint64_t fileSize);

			MOHPC_EXPORTS uint64_t getExpectedSize() const;
			MOHPC_EXPORTS uint64_t getFileSize() const;

		public:
			const char* what() const override;

		private:
			uint64_t expectedSize;
			uint64_t fileSize;
		};

		/**
		 * Bad image file.
		 */
		class BadImage : public Base {};

		namespace JPEG
		{
			class Base : public ImageError::Base {};
		}

		namespace TGA
		{
			class Base : public ImageError::Base {};

			/**
			 * Only type 2 (RGB), 3 (gray), and 10 (RGB) TGA images supported.
			 */
			class UnsupportedType : public Base
			{
			public:
				UnsupportedType(uint8_t type);

				MOHPC_EXPORTS uint8_t getType() const;

			public:
				const char* what() const override;

			private:
				uint8_t type;
			};

			/**
			 * Colormaps not supported.
			 */
			class UnsupportedColormap : public Base
			{
			public:
				UnsupportedColormap(uint8_t colorMapType);

				MOHPC_EXPORTS uint8_t getColorMapType() const;

			public:
				const char* what() const override;

			private:
				uint8_t colorMapType;
			};

			/**
			 * Only 32 or 24 bit images supported (no colormaps).
			 */
			class UnsupportedPixelSize : public Base 
			{
			public:
				UnsupportedPixelSize(uint8_t pixelSize);

				MOHPC_EXPORTS uint8_t getPixelSize() const;

			public:
				const char* what() const override;

			private:
				uint8_t pixelSize;
			};

			/**
			 * Has an invalid image size.
			 */
			class InvalidSize : public Base
			{
			public:
				InvalidSize(uint16_t width, uint16_t height);

				MOHPC_EXPORTS uint16_t getWidth() const;
				MOHPC_EXPORTS uint16_t getHeight() const;

			public:
				const char* what() const override;

			private:
				uint16_t width;
				uint16_t height;
			};
		}

		namespace DDS
		{
			class Base : public ImageError::Base {};

			/**
			 * The DDS has bad header.
			 */
			class BadHeader : public Base
			{
			public:
				BadHeader(const uint8_t foundHeader[4]);

				MOHPC_EXPORTS const uint8_t* getHeader() const;

			public:
				const char* what() const override;

			private:
				uint8_t foundHeader[4];
			};

			/**
			 * DDS File has unsupported DXGI format.
			 */
			class UnsupportedDXGIFormat : public Base
			{
			public:
				UnsupportedDXGIFormat(uint32_t format);

				MOHPC_EXPORTS uint32_t getFormat() const;

			public:
				const char* what() const override;

			private:
				uint32_t format;
			};

			/**
			 * DDS File has unsupported DXGI format.
			 */
			class UnsupportedFourCCFormat : public Base
			{
			public:
				UnsupportedFourCCFormat(uint32_t format);

				MOHPC_EXPORTS uint32_t getFormat() const;

			public:
				const char* what() const override;

			private:
				uint32_t format;
			};

			/**
			 * DDS File has unsupported DXGI format.
			 */
			class UnsupportedRGBAFormat : public Base
			{
			public:
				UnsupportedRGBAFormat(uint32_t format);

				MOHPC_EXPORTS uint32_t getFormat() const;

			public:
				const char* what() const override;

			private:
				uint32_t format;
			};
		}
	}
}
