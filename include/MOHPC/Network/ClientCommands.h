#pragma once

#include "Commands.h"

namespace MOHPC
{
	namespace Network
	{
		class IClientConnectionlessHandler : public IConnectionlessCommand
		{
		public:
			IClientConnectionlessHandler(const char* inEventName);
		};

		extern class CommandList clientEvents;

		extern IClientConnectionlessHandler& getKeyEvent;
		extern IClientConnectionlessHandler& challengeResponseEvent;
		extern IClientConnectionlessHandler& connectResponseEvent;
		extern IClientConnectionlessHandler& dropErrorEvent;
	}
}
