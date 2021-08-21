#include <MOHPC/Network/Remote/SocketUdpDelay.h>

#include <chrono>
#include <random>
#include <thread>
#include <cstring>

using namespace MOHPC;
using namespace MOHPC::Network;

PacketContainer::PacketContainer()
	: buffer(nullptr)
	, size(0)
{
}

PacketContainer::PacketContainer(const NetAddrPtr& addressPtr, void* bufferPtr, size_t sizeVal, std::chrono::nanoseconds timeVal)
	: address(addressPtr)
	, buffer(bufferPtr)
	, size(sizeVal)
	, time(timeVal)
{
}

PacketContainer::PacketContainer(PacketContainer&& other)
	: address(std::move(other.address))
	, buffer(other.buffer)
	, size(other.size)
	, time(other.time)
{
	other.buffer = nullptr;
	other.size = 0;
}

PacketContainer& PacketContainer::operator=(PacketContainer&& other)
{
	if (buffer)
	{
		// free the existing buffer
		delete[] static_cast<uint8_t*>(buffer);
	}

	address = std::move(other.address);
	buffer = other.buffer;
	size = other.size;
	time = other.time;

	other.buffer = nullptr;
	other.size = 0;

	return *this;
}

PacketContainer::~PacketContainer()
{
	if (buffer)
	{
		delete[] static_cast<uint8_t*>(buffer);
	}
}

const MOHPC::Network::NetAddrPtr& PacketContainer::getAddress() const
{
	return address;
}

size_t PacketContainer::getBufferSize() const
{
	return size;
}

void* PacketContainer::getBuffer() const
{
	return buffer;
}

std::chrono::nanoseconds PacketContainer::getTime() const
{
	return time;
}

MOHPC_OBJECT_DEFINITION(UdpSocketSimLatency);

UdpSocketSimLatency::UdpSocketSimLatency(const IUdpSocketPtr& originalSocketPtr)
	: originalSocket(originalSocketPtr)
	, minLatency(0)
	, jittering(0)
	, enableOOD(false)
	, isThreadActive(true)
	, startTime(std::chrono::steady_clock::now())
	, socketIncomingThread(&UdpSocketSimLatency::incomingThread, this)
	, socketOutgoingThread(&UdpSocketSimLatency::outgoingThread, this)
{
}

UdpSocketSimLatency::~UdpSocketSimLatency()
{
	isThreadActive = false;
	incomingEvent.notify_all();
	outgoingEvent.notify_all();
	socketIncomingThread.join();
	socketOutgoingThread.join();
}

void UdpSocketSimLatency::setLatency(uint32_t latency)
{
	minLatency = latency;
}

void UdpSocketSimLatency::setLatencyVariance(uint32_t variance)
{
	jittering = variance;
}

void UdpSocketSimLatency::setOutOfOrder(bool enabled)
{
	enableOOD = enabled;
}

size_t UdpSocketSimLatency::send(const NetAddrPtr& to, const void* buf, size_t bufsize)
{
	{
		uint8_t* bufCopy = new uint8_t[bufsize];
		std::memcpy(bufCopy, buf, bufsize);

		std::unique_lock<std::mutex> lk(outboundMutex);
		if (!shouldSendOutOfOrder())
		{
			outbounds.emplace_back(to, bufCopy, bufsize, calculatePacketTime());
		}
		else
		{
			outbounds.insert(outbounds.end(), { to, bufCopy, bufsize, calculatePacketTime() });
		}
	}

	outgoingEvent.notify_all();
	return bufsize;
}

size_t UdpSocketSimLatency::receive(void* buf, size_t maxsize, NetAddrPtr& from)
{
	std::unique_lock<std::mutex> lk(incomingMutex);

	PacketContainer pktCon = std::move(waitIncoming(lk));

	// dequeue the packet
	incomings.pop_front();

	// latency wait
	waitFor(pktCon.getTime());

	const size_t bufferSize = pktCon.getBufferSize();
	const size_t copySize = std::min(bufferSize, maxsize);

	from = pktCon.getAddress();
	std::memcpy(buf, pktCon.getBuffer(), copySize);

	return copySize;
}

bool UdpSocketSimLatency::wait(uint64_t timeout)
{
	using namespace std::chrono;

	nanoseconds packetTime = nanoseconds();
	{
		std::unique_lock<std::mutex> lk(incomingMutex);
		PacketContainer* pktCon = waitIncoming(lk, milliseconds(timeout));

		if (pktCon)
		{
			// case where packets are pending but have latency
			packetTime = pktCon->getTime();
		}
	}

	if (packetTime == nanoseconds())
	{
		// no packet received
		return false;
	}

	// latency wait
	waitFor(packetTime, milliseconds(timeout));
	return true;
}

size_t UdpSocketSimLatency::dataCount()
{
	std::unique_lock<std::mutex> lk(incomingMutex);

	if (!incomings.empty())
	{
		using namespace std::chrono;
		time_point<steady_clock> currentTime = steady_clock::now();

		const PacketContainer& pktCon = incomings.front();
		if (currentTime >= startTime + pktCon.getTime())
		{
			// finally received the packet
			return pktCon.getBufferSize();
		}
	}

	// nothing
	return 0;
}

void* UdpSocketSimLatency::getRaw()
{
	return originalSocket->getRaw();
}

PacketContainer& UdpSocketSimLatency::waitIncoming(std::unique_lock<std::mutex>& lk)
{
	if (incomings.empty())
	{
		// wait for new packets
		incomingEvent.wait(lk);
	}

	return incomings.front();
}

PacketContainer* UdpSocketSimLatency::waitIncomingSafe(std::unique_lock<std::mutex>& lk)
{
	if (incomings.empty())
	{
		// wait for new packets
		incomingEvent.wait(lk);
	}

	if (incomings.empty())
	{
		// no packet
		return nullptr;
	}

	return &incomings.front();
}

PacketContainer* UdpSocketSimLatency::waitIncoming(std::unique_lock<std::mutex>& lk, std::chrono::nanoseconds timeout)
{
	using namespace std::chrono;

	if (incomings.empty())
	{
		// wait for new packets
		incomingEvent.wait_for(lk, timeout);
	}

	if (incomings.empty())
	{
		// no packet
		return nullptr;
	}

	return &incomings.front();
}

void UdpSocketSimLatency::waitFor(std::chrono::nanoseconds packetTime) const
{
	using namespace std::chrono;
	const nanoseconds waitTime = getWaitTime(packetTime);

	if (waitTime > nanoseconds())
	{
		// infinite wait time
		std::this_thread::sleep_for(waitTime);
	}
}

void UdpSocketSimLatency::waitFor(std::chrono::nanoseconds packetTime, std::chrono::nanoseconds maxTime) const
{
	using namespace std::chrono;
	const nanoseconds waitTime = getWaitTime(packetTime);

	if (waitTime > nanoseconds())
	{
		// with max wait time
		std::this_thread::sleep_for(std::min(maxTime, waitTime));
	}
}

std::chrono::nanoseconds UdpSocketSimLatency::getWaitTime(std::chrono::nanoseconds packetTime) const
{
	using namespace std::chrono;
	const time_point<steady_clock> currentTime = steady_clock::now();
	const nanoseconds currentTimeDelta = currentTime - startTime;

	if (currentTimeDelta < packetTime)
	{
		// simulate time wait
		return packetTime - currentTimeDelta;
	}

	return nanoseconds();
}

bool UdpSocketSimLatency::shouldSendOutOfOrder() const
{
	if (!enableOOD) return false;
	
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> dist(0.f, 1.f);

	// send out of order
	return dist(gen) >= 0.5f;
}

std::chrono::nanoseconds UdpSocketSimLatency::calculatePacketTime() const
{
	using namespace std::chrono;
	return calculatePacketTime(steady_clock::now());
}

std::chrono::nanoseconds UdpSocketSimLatency::calculatePacketTime(std::chrono::time_point<std::chrono::steady_clock> currentTime) const
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<uint32_t> dist(0, jittering);

	using namespace std::chrono;
	return currentTime - startTime + milliseconds(minLatency + dist(gen));
}

size_t UdpSocketSimLatency::getSocketDataCount()
{
	return originalSocket->dataCount();
}

void UdpSocketSimLatency::incomingThread()
{
	// socket thread loop
	while (isThreadActive)
	{
		for (;;)
		{
			const bool hasPending = originalSocket->wait(100);
			if (!hasPending)
			{
				// nothing to process
				break;
			}

			const size_t dataSize = getSocketDataCount();
			if (dataSize)
			{

				void* buf = new uint8_t[dataSize];
				NetAddrPtr fromAddr;
				originalSocket->receive(buf, dataSize, fromAddr);

				{
					std::unique_lock<std::mutex> lk(incomingMutex);
					incomings.emplace_back(fromAddr, buf, dataSize, calculatePacketTime());
				}
			}
			else
			{
				uint8_t emptyBuf;
				NetAddrPtr fromAddr;
				// received empty message
				originalSocket->receive(&emptyBuf, 1, fromAddr);

				{
					std::unique_lock<std::mutex> lk(incomingMutex);
					incomings.emplace_back(fromAddr, nullptr, 0, calculatePacketTime());
				}
			}

			// notify about the incoming event
			incomingEvent.notify_all();
		}
	}
}

void UdpSocketSimLatency::outgoingThread()
{
	// socket thread loop
	while (isThreadActive)
	{
		PacketContainer pktCon;
		bool processPacket = false;
		{
			// wait for send events
			std::unique_lock<std::mutex> lk(outboundMutex);
			if (outbounds.empty())
			{
				// wait for outgoing packets
				outgoingEvent.wait(lk);
			}

			if (!outbounds.empty())
			{
				processPacket = true;
				pktCon = std::move(outbounds.front());
				outbounds.pop_front();
			}
		}

		if (processPacket)
		{
			using namespace std::chrono;
			const time_point<steady_clock> currentTime = steady_clock::now();

			waitFor(pktCon.getTime());

			// can now send it
			originalSocket->send(pktCon.getAddress(), pktCon.getBuffer(), pktCon.getBufferSize());
		}
	}
}

MOHPC_OBJECT_DEFINITION(UdpSocketSimLoss);

UdpSocketSimLoss::UdpSocketSimLoss(const IUdpSocketPtr& originalSocketPtr)
	: originalSocket(originalSocketPtr)
	, inboundLossAlpha(0.f)
	, outboundLossAlpha(0.f)
	, dupAlpha(0.f)
	, mustProcessIncoming(false)
{
}

UdpSocketSimLoss::~UdpSocketSimLoss()
{
}

size_t UdpSocketSimLoss::send(const NetAddrPtr& to, const void* buf, size_t bufsize)
{
	if (shouldDropOutbound())
	{
		// don't send anything
		return bufsize;
	}

	const size_t sent = originalSocket->send(to, buf, bufsize);
	if (shouldDuplicate())
	{
		// send it a second time!
		originalSocket->send(to, buf, bufsize);
	}

	return sent;
}

size_t UdpSocketSimLoss::receive(void* buf, size_t maxsize, NetAddrPtr& from)
{
	size_t receivedLen = 0;
	bool packetAvailable = false;

	while (!packetAvailable)
	{
		if (shouldDropIncoming())
		{
			// don't care
			dropIncoming();
		}
		else
		{
			receivedLen = originalSocket->receive(buf, maxsize, from);
			packetAvailable = true;
			mustProcessIncoming = false;
		}
	}

	return receivedLen;
}

bool UdpSocketSimLoss::wait(uint64_t timeout)
{
	using namespace std::chrono;
	const time_point<steady_clock> startTime = steady_clock::now();

	nanoseconds timeRemaining = duration_cast<nanoseconds>(milliseconds(timeout));

	while (!mustProcessIncoming)
	{
		const bool hasPendingData = originalSocket->wait(duration_cast<milliseconds>(timeRemaining).count());

		if (!shouldDropIncoming())
		{
			// force process the packet
			if (hasPendingData) {
				mustProcessIncoming = true;
			}
			break;
		}
		else
		{
			dropIncoming();

			time_point<steady_clock> currentTime = steady_clock::now();
			if (currentTime >= startTime + timeRemaining)
			{
				// no more time available to wait
				break;
			}
			
			timeRemaining -= currentTime - startTime;
		}
	}

	return mustProcessIncoming;
}

size_t UdpSocketSimLoss::dataCount()
{
	size_t count = originalSocket->dataCount();
	while (!mustProcessIncoming)
	{
		if (!shouldDropIncoming())
		{
			// force process the packet
			if (count > 0) {
				mustProcessIncoming = true;
			}
			break;
		}
		else
		{
			dropIncoming();
		}

		count = originalSocket->dataCount();
	}

	return count;
}

void* UdpSocketSimLoss::getRaw()
{
	return originalSocket->getRaw();
}

void UdpSocketSimLoss::setInboundPacketLossAlpha(float alpha)
{
	inboundLossAlpha = alpha;
}

void UdpSocketSimLoss::setOutboundPacketLossAlpha(float alpha)
{
	outboundLossAlpha = alpha;
}

void UdpSocketSimLoss::setDuplicateAlpha(float alpha)
{
	dupAlpha = alpha;
}

void UdpSocketSimLoss::dropIncoming()
{
	// don't receive anything actually
	uint8_t emptyBuf = 0;
	NetAddrPtr fromTmp;
	originalSocket->receive(&emptyBuf, 1, fromTmp);
}

bool UdpSocketSimLoss::shouldDropIncoming()
{
	if (mustProcessIncoming) {
		return false;
	}

	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> dist(0.f, 1.f);

	return inboundLossAlpha > 0.f && dist(gen) >= 1.f - inboundLossAlpha;
}

bool UdpSocketSimLoss::shouldDropOutbound()
{
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> dist(0.f, 1.f);

	return outboundLossAlpha > 0.f && dist(gen) >= 1.f - outboundLossAlpha;
}

bool UdpSocketSimLoss::shouldDuplicate()
{
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> dist(0.f, 1.f);

	return dupAlpha > 0.f && dist(gen) >= 1.f - dupAlpha;
}
