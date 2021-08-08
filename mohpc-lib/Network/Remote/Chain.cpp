#include <MOHPC/Network/Remote/Chain.h>

using namespace MOHPC;
using namespace MOHPC::Network;

IChain::IChain()
{
}

IChain::~IChain()
{
}

void IChain::setNext(const IChainPtr& nextPtr)
{
	next = nextPtr;
}

const MOHPC::Network::IChainPtr& IChain::getNext() const
{
	return next;
}
