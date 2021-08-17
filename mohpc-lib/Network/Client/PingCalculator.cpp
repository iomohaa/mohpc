#include <MOHPC/Network/Client/PingCalculator.h>
#include <MOHPC/Network/Client/ServerConnection.h>

using namespace MOHPC;
using namespace MOHPC::Network;

MOHPC_OBJECT_DEFINITION(PingCalculator);

pingPacket_t::pingPacket_t()
	: lastPacketTime(tickTime_t())
	, lastSnapTime(tickTime_t())
{

}

PingCalculator::PingCalculator(const SharedPtr<ServerConnection>& connectionPtr)
	: connection(connectionPtr)
	, latency(0)
	, currentPacket(0)
{
	using namespace std::placeholders;
	packetHandle = connectionPtr->getHandlerList().postWritePacketHandler.add(std::bind(&PingCalculator::postWritePacket, this, _1, _2));
	snapReceivedHandle = connectionPtr->getSnapshotManager().getHandlers().snapshotParsedHandler.add(std::bind(&PingCalculator::snapReceived, this, _1));
}

PingCalculator::~PingCalculator()
{
	if (!connection.expired())
	{
		// remove the callback
		const ServerConnectionPtr ptr = connection.lock();
		ptr->getHandlerList().postWritePacketHandler.remove(packetHandle);
		ptr->getSnapshotManager().getHandlers().snapshotParsedHandler.remove(snapReceivedHandle);
	}
}

void PingCalculator::postWritePacket(const ClientTime& time, uint32_t sequenceNum)
{
	using namespace std::chrono;
	const tickTime_t currentTime = tickClock_t::now();

	const pingPacket_t& lastPacket = getLastPacket();
	if (lastPacket.lastPacketTime == tickTime_t() || currentTime >= lastPacket.lastPacketTime + getMinDeltaTime())
	{
		pingPacket_t& packet = createPacket();
		packet.lastPacketTime = currentTime;
		packet.lastSnapTime = time_cast<tickTime_t>(time.getOldRemoteTime());
	}
}

void PingCalculator::snapReceived(const rawSnapshot_t& snap)
{
	for (size_t i = 0; i < getNumPackets(); ++i)
	{
		const size_t packetNum = currentPacket - 1 - i;
		pingPacket_t& packet = getPacket(packetNum);
		const tickTime_t commandTime = time_cast<tickTime_t>(snap.ps.getCommandTime());
		if (commandTime >= packet.lastSnapTime)
		{
			const tickTime_t currentTime = tickClock_t::now();

			latency = currentTime - packet.lastPacketTime;
			break;
		}
	}
}

deltaTime_t PingCalculator::getLatency() const
{
	return latency;
}

const pingPacket_t& PingCalculator::getPacket(size_t index) const
{
	return packets[index % getNumPackets()];
}

pingPacket_t& PingCalculator::getPacket(size_t index)
{
	return packets[index % getNumPackets()];
}

const pingPacket_t& PingCalculator::getLastPacket() const
{
	return packets[(currentPacket - 1) % getNumPackets()];
}

pingPacket_t& PingCalculator::createPacket()
{
	return packets[(currentPacket++) % getNumPackets()];
}

size_t PingCalculator::getNumPackets() const
{
	return sizeof(packets) / sizeof(packets[0]);
}

deltaTime_t PingCalculator::getMinDeltaTime() const
{
	using namespace std::chrono;
	return deltaTime_t(milliseconds(1000 / getNumPackets()));
}
