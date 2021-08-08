#pragma once

#include "../NetGlobal.h"
#include "../../Utility/SharedPtr.h"

namespace MOHPC
{
class IMessageStream;

namespace Network
{
	using IChainPtr = SharedPtr<class IChain>;
	class IChain
	{
	public:
		IChain();
		virtual ~IChain();

		virtual void handleTransmit(IMessageStream& stream) = 0;
		virtual void handleReceive(IMessageStream& stream) = 0;
		void setNext(const IChainPtr& nextPtr);

	protected:
		const IChainPtr& getNext() const;

	private:
		IChainPtr next;
	};
}
}
