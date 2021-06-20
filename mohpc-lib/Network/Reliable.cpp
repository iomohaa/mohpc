#include <MOHPC/Network/Reliable.h>

using namespace MOHPC;
using namespace Network;

IAbstractSequence::~IAbstractSequence()
{
}

IReliableSequence::IReliableSequence()
	: reliableSequence(0)
	, reliableAcknowledge(0)
{
}

void IReliableSequence::addCommand(const char* command)
{
	const size_t maxReliableCommands = getMaxElements();
	if (reliableAcknowledge + maxReliableCommands < reliableSequence)
	{
		// FIXME: throw?
		return;
	}

	++reliableSequence;
	const size_t index = reliableSequence % maxReliableCommands;

	set(reliableSequence, command);
}

rsequence_t IReliableSequence::getReliableSequence() const
{
	return reliableSequence;
}

rsequence_t IReliableSequence::getReliableAcknowledge() const
{
	return reliableAcknowledge;
}

const char* IReliableSequence::getSequence(rsequence_t index) const
{
	return get(index % getMaxElements());
}

const char* IReliableSequence::getLastSequence() const
{
	return getSequence(reliableSequence);
}

const char* IReliableSequence::getLastAcknowledgedSequence() const
{
	return getSequence(reliableAcknowledge);
}

void IReliableSequence::updateAcknowledge(rsequence_t acknowledged)
{
	reliableAcknowledge = acknowledged;
	if (reliableAcknowledge + getMaxElements() < reliableSequence) {
		reliableAcknowledge = reliableSequence;
	}
}

ICommandSequence::ICommandSequence()
	: remoteCommandSequence(0)
{
}

void ICommandSequence::addCommand(const char* command, rsequence_t sequence)
{
	if (remoteCommandSequence >= sequence) {
		return;
	}

	remoteCommandSequence = sequence;

	const size_t maxRemoteCommands = getMaxElements();
	const uint32_t index = sequence % maxRemoteCommands;

	set(index, command);
}

rsequence_t ICommandSequence::getCommandSequence() const
{
	return remoteCommandSequence;
}

const char* ICommandSequence::getSequence(rsequence_t index) const
{
	return get(index % getMaxElements());
}

const char* ICommandSequence::getLastSequence() const
{
	return getSequence(remoteCommandSequence);
}

void ICommandSequence::setSequence(rsequence_t newSequence)
{
	remoteCommandSequence = newSequence;
}
