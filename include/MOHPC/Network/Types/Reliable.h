#pragma once

#include "../NetGlobal.h"

#include <cstdint>
#include <cstddef>

namespace MOHPC
{
namespace Network
{

/** Remote sequence type. */
using rsequence_t = uint32_t;

class IAbstractSequence
{
public:
	virtual ~IAbstractSequence();

	virtual size_t getMaxElements() const = 0;
	virtual const char* get(rsequence_t index) const = 0;

protected:
	virtual void set(rsequence_t index, const char* command) = 0;
};

class IReliableSequence : public IAbstractSequence
{
public:
	IReliableSequence();

	MOHPC_NET_EXPORTS void addCommand(const char* command);
	MOHPC_NET_EXPORTS const char* getSequence(rsequence_t index) const;
	MOHPC_NET_EXPORTS rsequence_t getReliableSequence() const;
	MOHPC_NET_EXPORTS rsequence_t getReliableAcknowledge() const;
	MOHPC_NET_EXPORTS const char* getLastSequence() const;
	MOHPC_NET_EXPORTS const char* getLastAcknowledgedSequence() const;
	MOHPC_NET_EXPORTS void updateAcknowledge(rsequence_t acknowledged);

private:
	rsequence_t reliableSequence;
	rsequence_t reliableAcknowledge;
};
using IReliableSequencePtr = IReliableSequence;

class ICommandSequence : public IAbstractSequence
{
public:
	ICommandSequence();

	MOHPC_NET_EXPORTS void addCommand(const char* command, rsequence_t sequence);
	MOHPC_NET_EXPORTS const char* getSequence(rsequence_t index) const;
	MOHPC_NET_EXPORTS rsequence_t getCommandSequence() const;
	MOHPC_NET_EXPORTS const char* getLastSequence() const;
	MOHPC_NET_EXPORTS void setSequence(rsequence_t newSequence);

private:
	rsequence_t remoteCommandSequence;
};
using ICommandSequencePtr = ICommandSequence;

}
}