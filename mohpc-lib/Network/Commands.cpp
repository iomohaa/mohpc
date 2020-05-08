#include <MOHPC/Network/Commands.h>

using namespace MOHPC;
using namespace Network;

ICommand::ICommand(class CommandList& eventList)
	: next(nullptr)
	, prev(nullptr)
	, id(0)
{
	eventList.add(this);
}

ICommand::~ICommand()
{
	if (next) next->prev = prev;
	if (prev) prev->next = next;
}

ICommand* ICommand::getNext() const
{
	return next;
}

size_t ICommand::getId() const
{
	return id;
}

IConnectionlessCommand::IConnectionlessCommand(CommandList& eventList, const char* inEventName)
	: ICommand(eventList)
	, eventName(inEventName)
{}

CommandList::CommandList()
	: last(nullptr)
{
}

void CommandList::add(ICommand* cmd)
{
	if (!head) head = cmd;
	cmd->prev = last;

	if (last)
	{
		cmd->id = last->id + 1;
		last->next = cmd;
	}

	last = cmd;
}

ICommand* CommandList::getHead() const
{
	return head;
}

size_t CommandList::getNumCommands() const
{
	return last ? (last->id + 1) : 0;
}
