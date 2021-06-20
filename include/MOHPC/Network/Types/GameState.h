#pragma once

#include "../NetGlobal.h"
#include "../Types.h"
#include "Configstring.h"

#include <cstddef>

namespace MOHPC
{
class entityState_t;

namespace Network
{
	using baselineNum_t = uint32_t;

	/**
	 * Contains an array of base entities, used for diffing over the network.
	 */
	class BaseLines
	{
	public:
		/** Initialize the baseline, empty */
		MOHPC_NET_EXPORTS BaseLines();
		/**
		 * Initialize the fixed-size baselines with the maximum
		 * number of elements.
		 *
		 * @param maxBaselines maximum number of baseline entities
		 */
		MOHPC_NET_EXPORTS BaseLines(baselineNum_t maxBaselines);
		MOHPC_NET_EXPORTS ~BaseLines();

		/**
		 * Return the baseline at the specified index.
		 *
		 * @param index The baseline index.
		 */
		MOHPC_NET_EXPORTS const entityState_t& getBaseline(baselineNum_t index) const;
		MOHPC_NET_EXPORTS entityState_t& getBaseline(baselineNum_t index);

		/** Return the maximum number of baselines. */
		MOHPC_NET_EXPORTS baselineNum_t getMaxBaselines() const;

		/** Empty the baseline list. */
		MOHPC_NET_EXPORTS void reset();

	private:
		entityState_t* entityBaselines;
		baselineNum_t maxBaselines;
	};

	/**
	 * The gameState handles config-strings and base entities.
	 */
	struct gameState_t
	{
	public:
		MOHPC_NET_EXPORTS gameState_t();
		MOHPC_NET_EXPORTS gameState_t(const size_t numConfigStrings, const size_t maxChars, const baselineNum_t maxBaselines);
		MOHPC_NET_EXPORTS ~gameState_t();

		MOHPC_NET_EXPORTS ConfigStringManager& getConfigstringManager();
		MOHPC_NET_EXPORTS const ConfigStringManager& getConfigstringManager() const;
		MOHPC_NET_EXPORTS BaseLines& getEntityBaselines();
		MOHPC_NET_EXPORTS const BaseLines& getEntityBaselines() const;

		MOHPC_NET_EXPORTS void reset();

	private:
		ConfigStringManager csMan;
		BaseLines entityBaselines;
	};

	namespace GameStateErrors
	{
		class Base : public NetworkException {};

		/**
		 * Invalid baseline entity number while parsing gamestate.
		 */
		class BaselineOutOfRangeException : public Base
		{
		public:
			BaselineOutOfRangeException(baselineNum_t inBaselineNum);

			MOHPC_NET_EXPORTS baselineNum_t getBaselineNum() const;
			MOHPC_NET_EXPORTS str what() const override;

		private:
			baselineNum_t baselineNum;
		};
	}
}
}