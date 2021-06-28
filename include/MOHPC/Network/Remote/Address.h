#pragma once

#include "../NetGlobal.h"
#include "../NetObject.h"
#include "../../Common/str.h"

#include <cstdint>
#include <cstddef>

namespace MOHPC
{
namespace Network
{
	/**
	 * Abstract interface for internet address.
	 */
	struct NetAddr
	{
		uint16_t port;

	public:
		MOHPC_NET_EXPORTS NetAddr();

		/**
		 * Set the address port.
		 *
		 * @param value Port.
		 */
		MOHPC_NET_EXPORTS void setPort(uint16_t value);
		/** Return this address port. */
		MOHPC_NET_EXPORTS uint16_t getPort() const;

		/** Return whether or not this address equals to another one. */
		MOHPC_NET_EXPORTS bool operator==(const NetAddr& other) const;
		/** Return whether or not this address differs from another one. */
		MOHPC_NET_EXPORTS bool operator!=(const NetAddr& other) const;

	public:
		/** Return the address size. */
		virtual size_t getAddrSize() const = 0;
		/** Return the address array. */
		virtual const uint8_t* getAddress() const = 0;
		/** Return the address parsed as a string. */
		virtual str asString() const = 0;
	};
	using NetAddrPtr = SharedPtr<NetAddr>;

	/**
	 * IPv4 internet address.
	 */
	struct NetAddr4 : public NetAddr
	{
		MOHPC_NET_OBJECT_DECLARATION(NetAddr4);
	public:
		MOHPC_NET_EXPORTS NetAddr4();

		/** Set the IPv4 address. */
		MOHPC_NET_EXPORTS void setIp(uint8_t a, uint8_t b, uint8_t c, uint8_t d);

	public:
		size_t getAddrSize() const override;
		const uint8_t* getAddress() const override;
		virtual str asString() const override;

	public:
		uint8_t ip[4];
	};
	using NetAddr4Ptr = SharedPtr<NetAddr4>;

	/**
	 * IPv6 internet address.
	 */
	struct NetAddr6 : public NetAddr
	{
		MOHPC_NET_OBJECT_DECLARATION(NetAddr6);

	public:
		MOHPC_NET_EXPORTS NetAddr6();

		/** Set the IPv6 address. */
		MOHPC_NET_EXPORTS void setIp(uint16_t a, uint16_t b, uint16_t c, uint16_t d, uint16_t e, uint16_t f, uint16_t g, uint16_t h);

	public:
		size_t getAddrSize() const override;
		const uint8_t* getAddress() const override;
		virtual str asString() const override;

	public:
		uint8_t ip[16];
	};
	using NetAddr6Ptr = SharedPtr<NetAddr6>;
}
}