#pragma once

#include "Reliable.h"
#include "../../Common/str.h"

namespace MOHPC
{
namespace Network
{
	template<size_t MAX_RELIABLE_COMMANDS, size_t RELIABLE_COMMAND_SIZE>
	class SequenceTemplate : public IReliableSequence
	{
	public:
		void set(rsequence_t index, const char* command) override;
		const char* get(rsequence_t index) const override;
		size_t getMaxElements() const override;

	private:
		char reliableCommands[MAX_RELIABLE_COMMANDS][RELIABLE_COMMAND_SIZE];
	};

	template<size_t MAX_RELIABLE_COMMANDS, size_t RELIABLE_COMMAND_SIZE>
	class RemoteCommandSequenceTemplate : public ICommandSequence
	{
	public:
		void set(rsequence_t index, const char* command) override;
		const char* get(rsequence_t index) const override;
		size_t getMaxElements() const override;

	private:
		char serverCommands[MAX_RELIABLE_COMMANDS][RELIABLE_COMMAND_SIZE];
	};

	template<size_t MAX_RELIABLE_COMMANDS, size_t RELIABLE_COMMAND_SIZE>
	void SequenceTemplate<MAX_RELIABLE_COMMANDS, RELIABLE_COMMAND_SIZE>::set(rsequence_t index, const char* command)
	{
		str::copyn(reliableCommands[index], command, RELIABLE_COMMAND_SIZE);
	}

	template<size_t MAX_RELIABLE_COMMANDS, size_t RELIABLE_COMMAND_SIZE>
	const char* SequenceTemplate<MAX_RELIABLE_COMMANDS, RELIABLE_COMMAND_SIZE>::get(rsequence_t index) const
	{
		return reliableCommands[index];
	}

	template<size_t MAX_RELIABLE_COMMANDS, size_t RELIABLE_COMMAND_SIZE>
	size_t SequenceTemplate<MAX_RELIABLE_COMMANDS, RELIABLE_COMMAND_SIZE>::getMaxElements() const
	{
		return RELIABLE_COMMAND_SIZE;
	}

	template<size_t MAX_RELIABLE_COMMANDS, size_t RELIABLE_COMMAND_SIZE>
	void RemoteCommandSequenceTemplate<MAX_RELIABLE_COMMANDS, RELIABLE_COMMAND_SIZE>::set(rsequence_t index, const char* command)
	{
		str::copyn(serverCommands[index], command, RELIABLE_COMMAND_SIZE);
	}

	template<size_t MAX_RELIABLE_COMMANDS, size_t RELIABLE_COMMAND_SIZE>
	const char* RemoteCommandSequenceTemplate<MAX_RELIABLE_COMMANDS, RELIABLE_COMMAND_SIZE>::get(rsequence_t index) const
	{
		return serverCommands[index];
	}

	template<size_t MAX_RELIABLE_COMMANDS, size_t RELIABLE_COMMAND_SIZE>
	size_t RemoteCommandSequenceTemplate<MAX_RELIABLE_COMMANDS, RELIABLE_COMMAND_SIZE>::getMaxElements() const
	{
		return RELIABLE_COMMAND_SIZE;
	}
}
}