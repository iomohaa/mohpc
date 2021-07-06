#pragma once

#include "Base.h"
#include "../Hud.h"

namespace MOHPC
{
namespace Network
{
	namespace CGame
	{
		namespace Messages
		{
			class IHUD : public IMessage
			{
			public:
				virtual void SetShader(uint8_t index, const char* name) = 0;
				virtual void SetAlignment(uint8_t index, horizontalAlign_e horizontalAlign, verticalAlign_e verticalAlign) = 0;
				virtual void SetRect(uint8_t index, uint16_t x, uint16_t y, uint16_t width, uint16_t height) = 0;
				virtual void SetVirtualScreen(uint8_t index, bool virtualScreen) = 0;
				virtual void SetColor(uint8_t index, const Vector& color) = 0;
				virtual void SetAlpha(uint8_t index, float alpha) = 0;
				virtual void SetString(uint8_t index, const char* string) = 0;
				virtual void SetFont(uint8_t index, const char* fontName) = 0;
			};
		}
	}
}
}
