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
	}
}
}