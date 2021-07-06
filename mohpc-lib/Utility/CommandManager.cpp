#include <MOHPC/Utility/CommandManager.h>
#include <MOHPC/Utility/TokenParser.h>

using namespace MOHPC;

CommandManager::CommandManager()
{
}

void CommandManager::reserve(size_t num)
{
	commands.Resize(commands.NumObjects() + num);
}

void CommandManager::add(const char* name, ICommand* command)
{
	commands.AddObject(CommandData(name, command));
}

void CommandManager::process(const char* commandString)
{
	TokenParser tokenized(commandString);

	TokenParser::scriptmarker_t mark;
	tokenized.MarkPosition(&mark);

	const char* command = tokenized.GetToken(false);
	// find the command in list
	const size_t numCmds = commands.NumObjects();
	for (size_t i = 0; i < numCmds; ++i)
	{
		const CommandData& cmd = commands[i];
		if (!str::icmp(command, cmd.getName()))
		{
			// Call the correct function
			cmd.getCommand()->execute(tokenized);
			break;
		}
	}

	tokenized.RestorePosition(&mark);

	command = tokenized.GetToken(false);
	handlers().commandNotify.broadcast(command, tokenized);
}

CommandManager::HandlerList& CommandManager::handlers()
{
	return handlerList;
}

const CommandManager::HandlerList& CommandManager::handlers() const
{
	return handlerList;
}

CommandHandler::CommandHandler(std::function<void(TokenParser&)>&& functionMv)
	: function(std::move(functionMv))
{
}

void CommandHandler::execute(TokenParser& tokenized)
{
	function(tokenized);
}

CommandManager::CommandData::CommandData(const char* nameValue, ICommand* commandPtr)
	: name(nameValue)
	, command(commandPtr)
{
}

const char* CommandManager::CommandData::getName() const
{
	return name;
}

ICommand* CommandManager::CommandData::getCommand() const
{
	return command;
}
