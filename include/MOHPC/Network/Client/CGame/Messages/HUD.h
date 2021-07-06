#pragma once

#include "Base.h"
#include "../MessageInterfaces/IHUD.h"

namespace MOHPC
{
namespace Network
{
	namespace CGame
	{
		namespace Handlers
		{
			/**
			 * Set the shader to HUD index.
			 *
			 * @param	index	HUD index.
			 * @param	name	HUD name.
			 */
			struct HudDraw_Shader : public HandlerNotifyBase<void(uint8_t index, const char* name)> {};
			/**
			 * Set the alignment for HUD.
			 *
			 * @param	index				HUD index.
			 * @param	horizontalAlign		Horizontal alignment on screen.
			 * @param	verticalAlign		Vertical alignment on screen.
			 */
			struct HudDraw_Align : public HandlerNotifyBase<void(uint8_t index, horizontalAlign_e horizontalAlign, verticalAlign_e verticalAlign)> {};

			/**
			 * Set the HUD rect.
			 *
			 * @param	index	HUD index.
			 * @param	x		X position on screen.
			 * @param	y		Y position on screen.
			 * @param	width	Width of the element.
			 * @param	height	Height of the element.
			 */
			struct HudDraw_Rect : public HandlerNotifyBase<void(uint8_t index, uint16_t x, uint16_t y, uint16_t width, uint16_t height)> {};

			/**
			 * Specify if the HUD is virtual screen (rect relative to 640x480).
			 *
			 * @param	index			HUD index.
			 * @param	virtualScreen	True if the HUD is virtual screen.
			 */
			struct HudDraw_VirtualScreen : public HandlerNotifyBase<void(uint8_t index, bool virtualScreen)> {};

			/**
			 * Specify the color of the HUD.
			 *
			 * @param	index	HUD index.
			 * @param	color	[[0...1]...] RGB color of the HUD.
			 */
			struct HudDraw_Color : public HandlerNotifyBase<void(uint8_t index, const Vector& color)> {};

			/**
			 * Specify the alpha of the HUD.
			 *
			 * @param	index	HUD index.
			 * @arap	alpha	[0...1] Alpha.
			 */
			struct HudDraw_Alpha : public HandlerNotifyBase<void(uint8_t index, float alpha)> {};

			/**
			 * Set the string of the HUD element.
			 *
			 * @param	index	HUD index.
			 * @param	string	string value.
			 */
			struct HudDraw_String : public HandlerNotifyBase<void(uint8_t index, const char* string)> {};

			/**
			 * Set the font to use for the HUD (only if it has a string).
			 *
			 * @param	index		HUD index.
			 * @param	fontName	Name of the font.
			 */
			struct HudDraw_Font : public HandlerNotifyBase<void(uint8_t index, const char* fontName)> {};
		}

		namespace Messages
		{
			class HUDImpl : public IHUD
			{
			public:
				void SetShader(uint8_t index, const char* name) override;
				void SetAlignment(uint8_t index, horizontalAlign_e horizontalAlign, verticalAlign_e verticalAlign) override;
				void SetRect(uint8_t index, uint16_t x, uint16_t y, uint16_t width, uint16_t height) override;
				void SetVirtualScreen(uint8_t index, bool virtualScreen) override;
				void SetColor(uint8_t index, const Vector& color) override;
				void SetAlpha(uint8_t index, float alpha) override;
				void SetString(uint8_t index, const char* string) override;
				void SetFont(uint8_t index, const char* fontName) override;

			public:
				FunctionList<Handlers::HudDraw_Align> setAlignmentHandler;
				FunctionList<Handlers::HudDraw_Alpha> setAlphaHandler;
				FunctionList<Handlers::HudDraw_Color> setColorHandler;
				FunctionList<Handlers::HudDraw_Font> setFontHandler;
				FunctionList<Handlers::HudDraw_Rect> setRectHandler;
				FunctionList<Handlers::HudDraw_Shader> setShaderHandler;
				FunctionList<Handlers::HudDraw_String> setStringHandler;
				FunctionList<Handlers::HudDraw_VirtualScreen> setVirtualScreenHandler;
			};
		}
	}
}
}
