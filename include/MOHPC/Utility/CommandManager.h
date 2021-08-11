#pragma once

#include "UtilityGlobal.h"
#include "HandlerList.h"

#include <set>
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

	/**
	 * Command interface.
	 */
	class ICommand
	{
	public:
		virtual ~ICommand() = default;
		virtual void execute(TokenParser& tokenized) = 0;
	};

	
	template<typename T, void (T::*exec)(TokenParser&)>
	class CommandTemplate : public ICommand
	{
	public:
		CommandTemplate(T& instancePtr)
			: instance(instancePtr)
		{
		}

		void execute(TokenParser& tokenized) override
		{
			(instance.*exec)(tokenized);
		}

	private:
		T& instance;
	};

	class CommandHandler : public ICommand
	{
	public:
		CommandHandler(std::function<void(TokenParser&)>&& functionMv);

		void execute(TokenParser& tokenized) override;

	private:
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

		class CommandData
		{
		public:
			CommandData(const char* name);
			CommandData(ICommand* commandPtr);
			CommandData(const char* nameValue, ICommand* commandPtr);

			const char* getName() const;
			ICommand* getCommand() const;

		public:
			const char* name;
			ICommand* command;
		};

		class CommandDataLess
		{
		public:
			bool operator()(const CommandData& lhs, const CommandData& rhs) const;
		};

	public:
		MOHPC_UTILITY_EXPORTS CommandManager();

		/** Add an handler for the specified command. */
		MOHPC_UTILITY_EXPORTS void add(const char* name, ICommand* command);
		MOHPC_UTILITY_EXPORTS void remove(ICommand* command);
		MOHPC_UTILITY_EXPORTS void remove(const char* name);
		MOHPC_UTILITY_EXPORTS void reserve(size_t num);

		MOHPC_UTILITY_EXPORTS void process(const char* commandString);

		MOHPC_UTILITY_EXPORTS HandlerList& handlers();
		MOHPC_UTILITY_EXPORTS const HandlerList& handlers() const;

	private:
		HandlerList handlerList;
		std::vector<CommandData> commands;
	};
}