#include <MOHPC/Utility/CommandManager.h>
#include <MOHPC/Utility/TokenParser.h>

using namespace MOHPC;

Command::Command(const char* cmdNameValue, std::function<void(TokenParser&)>&& functionMv)
	: cmdName(cmdNameValue)
	, function(std::move(functionMv))
{
}

const char* Command::getName() const
{
	return cmdName;
}

const std::function<void(TokenParser&)> Command::getFunction() const
{
	return function;
}

CommandManager::CommandManager()
{
}

void CommandManager::reserveCommands(size_t num)
{
	commands.Resize(commands.NumObjects() + num);
}

void CommandManager::addCommand(Command&& command)
{
	commands.AddObject(std::move(command));
}

void CommandManager::processCommand(TokenParser& tokenized)
{
	TokenParser::scriptmarker_t mark;
	tokenized.MarkPosition(&mark);

	const char* command = tokenized.GetToken(false);
	// find the command in list
	const size_t numCmds = commands.NumObjects();
	for (size_t i = 0; i < numCmds; ++i)
	{
		const Command& cmd = commands[i];
		if (!str::icmp(command, cmd.getName()))
		{
			// Call the correct function
			cmd.getFunction()(tokenized);
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
