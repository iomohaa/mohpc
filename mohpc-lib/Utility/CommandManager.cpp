#include <MOHPC/Utility/CommandManager.h>
#include <MOHPC/Utility/TokenParser.h>

using namespace MOHPC;

CommandManager::CommandManager()
{
}

void CommandManager::reserve(size_t num)
{
}

void CommandManager::add(const char* name, ICommand* command)
{
	commands.emplace_back(name, command);
}

void CommandManager::remove(ICommand* command)
{
	auto it = std::find_if(commands.begin(), commands.end(),
		[command](const CommandData& entry) -> bool
		{
			return entry.command == command;
		});

	if (it != commands.end()) commands.erase(it);
}

void CommandManager::remove(const char* name)
{
	auto it = std::find_if(commands.begin(), commands.end(),
		[name](const CommandData& entry) -> bool
		{
			return !strHelpers::icmp(entry.name, name);
		});

	if (it != commands.end()) commands.erase(it);
}

void CommandManager::process(const char* commandString)
{
	TokenParser tokenized(commandString);

	TokenParser::scriptmarker_t mark;
	tokenized.MarkPosition(&mark);

	const char* name = tokenized.GetToken(false);
	// find the command in list
	const auto cmdData = std::find_if(commands.begin(), commands.end(),
		[name](const CommandData& entry) -> bool
		{
			return !strHelpers::icmp(entry.name, name);
		});

	if (cmdData != commands.end())
	{
		// execute the specified command
		cmdData->getCommand()->execute(tokenized);
	}

	tokenized.RestorePosition(&mark);

	name = tokenized.GetToken(false);
	handlers().commandNotify.broadcast(name, tokenized);
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

CommandManager::CommandData::CommandData(const char* nameValue)
	: name(nameValue)
	, command(nullptr)
{
}

CommandManager::CommandData::CommandData(ICommand* commandPtr)
	: name(nullptr)
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

bool CommandManager::CommandDataLess::operator()(const CommandData& lhs, const CommandData& rhs) const
{
	if (lhs.getName() || rhs.getName())
	{
		return strHelpers::icmp(
			lhs.getName() ? lhs.getName() : "",
			rhs.getName() ? rhs.getName() : ""
		);
	}
	else if (lhs.getCommand() || rhs.getCommand())
	{
		return (size_t)lhs.getCommand() < (size_t)rhs.getCommand();
	}

	return false;
}
