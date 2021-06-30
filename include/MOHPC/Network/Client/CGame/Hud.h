#pragma once

#include "../../../Utility/HandlerList.h"

#include <cstdint>

namespace MOHPC
{
class Vector;

namespace Network
{
	namespace CGame
	{
		class Scoreboard;
		struct stats_t;

		enum class hudMessage_e : unsigned char
		{
			Yellow = 1,
			/** White message, shown top center with death messages. */
			ChatWhite,
			/** White message, shown top left, below compass. */
			White,
			/** Red message, shown top center with death messages. */
			ChatRed,
			/** (protocol version 15) Green message, shown top center with death messages. */
			ChatGreen,
			/** Maximum number of valid message type that can be sent by the server. */
			Max,
			/** Message printed in console instead of the HUD. */
			Console,
		};

		namespace Handlers
		{
			//=== Hud drawing functions

			/** HUD Alignment. */
			enum class horizontalAlign_e : unsigned char { left, center, right };
			enum class verticalAlign_e : unsigned char { top, center, bottom };

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

			/**
			 * Called to print a message on console.
			 *
			 * @param	type	Type of the message (see hudMessage_e).
			 * @param	text	Text to print.
			 */
			struct Print : public HandlerNotifyBase<void(hudMessage_e type, const char* text)> {};

			/**
			 * Called to print a message that is displayed on HUD, yellow color.
			 *
			 * @param	text	Text to print.
			 */
			struct HudPrint : public HandlerNotifyBase<void(const char* text)> {};

			/**
			 * Called from server after score has been parsed.
			 *
			 * @param	scores	Score data.
			 */
			struct ServerCommand_Scores : public HandlerNotifyBase<void(const Scoreboard& scores)> {};

			/**
			 * Called to display a stopwatch.
			 *
			 * @param	startTime	Start time of the stopwatch.
			 * @param	endTime		End time of the stopwatch.
			 */
			struct ServerCommand_Stopwatch : public HandlerNotifyBase<void(uint64_t startTime, uint64_t endTime)> {};

			/**
			 * The server notify clients when it experiences lags, such as hitches.
			 */
			struct ServerCommand_ServerLag : public HandlerNotifyBase<void()> {};
		}
	}
}
}