#pragma once

#include "../../Utility/ClassList.h"

#include <cstdint>

namespace MOHPC
{
namespace Network
{
	/**
	 * Protocol class singleton.
	 */
	template<typename T>
	class IProtocolSingleton : public ClassList<IProtocolSingleton<T>>
	{
	public:
		virtual ~IProtocolSingleton() = default;

		/**
		 * Retrieve the minimum and the maximum supported protocol version.
		 *
		 * @param minRange The minimum supported protocol version.
		 * @param maxRange The maximum supported protocol version.
		 */
		virtual void getProtocol(uint32_t& minRange, uint32_t& maxRange) const = 0;

		/**
		 * Retrieve the singleton that matches the protocol.
		 *
		 * @param protocol The protocol.
		 */
		static const T* get(uint32_t protocol);
	};

	template<typename T>
	const T* IProtocolSingleton<T>::get(uint32_t protocol)
	{
		for (const IProtocolSingleton* inst = getHead(); inst; inst = inst->getNext())
		{
			uint32_t min = 0, max = 0;
			inst->getProtocol(min, max);

			if (protocol >= min && protocol <= max)
			{
				// instantiate the class tied to the protocol
				return static_cast<const T*>(inst);
			}
		}

		// fallback to a default protocol
		for (const IProtocolSingleton* inst = getHead(); inst; inst = inst->getNext())
		{
			uint32_t min = 0, max = 0;
			inst->getProtocol(min, max);

			if (!min && !max)
			{
				return static_cast<const T*>(inst);
			}
		}

		return nullptr;
	}

	template<typename Base, uint32_t version>
	class ProtocolSingletonInherit : public Base
	{
	public:
		void getProtocol(uint32_t& minRange, uint32_t& maxRange) const override { minRange = maxRange = version; }
	};

	/**
	 * Class instantiator singleton based on a protocol version.
	 */
	template<typename Base>
	class IProtocolClassInstancier : public IProtocolSingleton<IProtocolClassInstancier<Base>>
	{
	public:
		/**
		 * Instantiate the abstract class.
		 */
		virtual Base* createInstance() const = 0;
	};

	/**
	 * Template for instantiating specific classes based on protocol version.
	 */
	template<typename T, typename Base, uint32_t minProtocolVersion, uint32_t maxProtocolVersion>
	class ProtocolClassInstancier_Template : public IProtocolClassInstancier<Base>
	{
	public:
		void getProtocol(uint32_t& minRange, uint32_t& maxRange) const { minRange = minProtocolVersion; maxRange = maxProtocolVersion; }
		Base* createInstance() const { return new T(); }
	};
}
}