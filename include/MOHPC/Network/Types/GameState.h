#pragma once

#include "../NetGlobal.h"
#include "../Types.h"
#include "../InfoTypes.h"
#include "Configstring.h"

#include <cstddef>

namespace MOHPC
{
class entityState_t;

namespace Network
{
	/**
	 * Contains an array of base entities, used for diffing over the network.
	 */
	class EntityList
	{
	public:
		/** Initialize the baseline, empty */
		MOHPC_NET_EXPORTS EntityList();
		/**
		 * Initialize the fixed-size baselines with the maximum
		 * number of elements.
		 *
		 * @param maxBaselines maximum number of baseline entities
		 */
		MOHPC_NET_EXPORTS EntityList(entityNum_t maxEntities);
		MOHPC_NET_EXPORTS ~EntityList();

		/**
		 * Return the baseline at the specified index.
		 *
		 * @param index The baseline index.
		 */
		MOHPC_NET_EXPORTS const entityState_t& getEntity(entityNum_t index) const;
		MOHPC_NET_EXPORTS entityState_t& getEntity(entityNum_t index);

		/** Return the maximum number of baselines. */
		MOHPC_NET_EXPORTS entityNum_t getMaxEntities() const;

		/** Empty the baseline list. */
		MOHPC_NET_EXPORTS void reset();

	private:
		entityState_t* entityList;
		entityNum_t maxEntities;
	};

	/**
	 * The gameState handles config-strings and base entities.
	 */
	struct gameState_t
	{
	public:
		MOHPC_NET_EXPORTS gameState_t();
		MOHPC_NET_EXPORTS gameState_t(const size_t numConfigStrings, const size_t maxChars, const entityNum_t maxBaselines);
		MOHPC_NET_EXPORTS ~gameState_t();

		MOHPC_NET_EXPORTS ConfigStringManager& getConfigstringManager();
		MOHPC_NET_EXPORTS const ConfigStringManager& getConfigstringManager() const;
		MOHPC_NET_EXPORTS EntityList& getEntityBaselines();
		MOHPC_NET_EXPORTS const EntityList& getEntityBaselines() const;

		MOHPC_NET_EXPORTS void reset();

	private:
		ConfigStringManager csMan;
		EntityList entityBaselines;
	};

	namespace GameStateErrors
	{
		class Base : public NetworkException {};

		/**
		 * Invalid baseline entity number while parsing gamestate.
		 */
		class EntityOutOfRangeException : public Base
		{
		public:
			EntityOutOfRangeException(entityNum_t inEntityNum);

			MOHPC_NET_EXPORTS entityNum_t getEntityNum() const;
			MOHPC_NET_EXPORTS str what() const override;

		private:
			entityNum_t entityNum;
		};
	}
}
}