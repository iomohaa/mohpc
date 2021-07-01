#pragma once

#include "UtilityGlobal.h"
#include "HandlerList.h"

#include <functional>
#include <cstdint>
#include <cstddef>

namespace MOHPC
{
	class TokenParser;

	namespace CommandHandlers
	{
		/**
		 * Called whenever a command occurs.
		 * Use this for general-purpose.
		 *
		 * @param	command	The command.
		 * @param	parser	Used to parse the command arguments.
		 */
		struct CommandNotify : public HandlerNotifyBase<void(const char* command, TokenParser& parser)> {};
	}

	class Command
	{
	public:
		Command(const char* cmdNameValue, std::function<void(TokenParser&)>&& functionMv);

		const char* getName() const;
		const std::function<void(TokenParser&)> getFunction() const;

	private:
		const char* cmdName;
		std::function<void(TokenParser&)> function;
	};

	/**
	 * Handles incoming server command in snapshots.
	 */
	class CommandManager
	{
	public:
		struct HandlerList
		{
			FunctionList<CommandHandlers::CommandNotify> commandNotify;
		};

	public:
		MOHPC_UTILITY_EXPORTS CommandManager();

		/** Add an handler for the specified command. */
		MOHPC_UTILITY_EXPORTS void addCommand(Command&& command);
		MOHPC_UTILITY_EXPORTS void reserveCommands(size_t num);

		MOHPC_UTILITY_EXPORTS void processCommand(const char* commandString);

		MOHPC_UTILITY_EXPORTS HandlerList& handlers();
		MOHPC_UTILITY_EXPORTS const HandlerList& handlers() const;

	private:
		HandlerList handlerList;
		mfuse::con::Container<Command> commands;
	};
}