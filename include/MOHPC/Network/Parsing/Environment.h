#pragma once

#include "../ProtocolSingleton.h"
#include "../../Common/str.h"

namespace MOHPC
{
namespace Network
{
	struct environment_t;

	namespace Parsing
	{
		class IEnvironment : public IProtocolSingleton<IEnvironment>
		{
		public:
			virtual void parseEnvironment(const char* string, environment_t& env) const = 0;
			virtual void parseSky(const char* string, environment_t& env) const = 0;
			virtual void writeEnvironment(const environment_t& env, str& out) const = 0;
			virtual void writeSky(const environment_t& env, str& out) const = 0;
		};
	}
}
}
