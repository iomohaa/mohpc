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

		/** HUD horizontal alignment. */
		enum class horizontalAlign_e : unsigned char
		{
			left,
			center,
			right
		};

		/** HUD vertical alignment. */
		enum class verticalAlign_e : unsigned char
		{
			top,
			center,
			bottom
		};

		namespace Handlers
		{
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