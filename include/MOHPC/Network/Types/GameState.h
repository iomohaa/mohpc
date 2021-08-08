#pragma once

#include "../NetGlobal.h"
#include "../Exception.h"
#include "../Types/Entity.h"
#include "Configstring.h"

#include <cstddef>

namespace MOHPC
{
namespace Network
{
	class entityState_t;

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
		MOHPC_NET_EXPORTS EntityList(EntityList&& other);
		MOHPC_NET_EXPORTS EntityList& operator=(EntityList&& other);
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
		void freeList();

	private:
		entityState_t* entityList;
		entityNum_t maxEntities;
	};

	struct mapInfo_t
	{
	public:
		mapInfo_t();
		mapInfo_t(uint32_t serverIdVal, uint32_t checksumFeedVal);

		/** Return the server id of the game state (represents an ID of the current map). */
		MOHPC_NET_EXPORTS uint32_t getServerId() const;

		/** Return the map checksum. */
		MOHPC_NET_EXPORTS uint32_t getChecksumFeed() const;

	private:
		uint32_t serverId;
		uint32_t checksumFeed;
	};

	/**
	 * The gameState handles config-strings and base entities.
	 */
	struct gameState_t
	{
	public:
		MOHPC_NET_EXPORTS gameState_t();
		MOHPC_NET_EXPORTS gameState_t(const size_t numConfigStrings, const size_t maxChars, const entityNum_t maxBaselines);
		MOHPC_NET_EXPORTS gameState_t(gameState_t&& other);
		MOHPC_NET_EXPORTS gameState_t& operator=(gameState_t&& other);
		MOHPC_NET_EXPORTS ~gameState_t();

		/** Return the config string manager. */
		MOHPC_NET_EXPORTS ConfigStringManager& getConfigstringManager();
		MOHPC_NET_EXPORTS const ConfigStringManager& getConfigstringManager() const;

		/** Return the list of initial entities, to be used as a delta reference. */
		MOHPC_NET_EXPORTS EntityList& getEntityBaselines();
		MOHPC_NET_EXPORTS const EntityList& getEntityBaselines() const;

		/** Return information about the current map. */
		MOHPC_NET_EXPORTS mapInfo_t& getMapInfo();
		MOHPC_NET_EXPORTS const mapInfo_t& getMapInfo() const;

		/** Reset the entire game state, reset data without freeing them. */
		MOHPC_NET_EXPORTS void reset();

	private:
		ConfigStringManager csMan;
		EntityList entityBaselines;
		mapInfo_t mapInfo;
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