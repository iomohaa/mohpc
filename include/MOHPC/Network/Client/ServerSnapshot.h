#pragma once

#include "../../Utility/HandlerList.h"
#include "../../Utility/TickTypes.h"
#include "../Types/Snapshot.h"
#include "../Types/Protocol.h"

#include "../Parsing/Entity.h"
#include "../Parsing/PlayerState.h"
#include "../Parsing/PVS.h"
#include "../Parsing/Snapshot.h"

#include "Packet.h"

#include <cstdint>
#include <cstddef>

namespace MOHPC
{
class MSG;

namespace Network
{
	static constexpr unsigned long MAX_PARSE_ENTITIES = 2048;

	namespace ClientHandlers
	{
		 /**
		  * Called when the current player state has been modified.
		  *
		  * @param	old		Previous player state (can be NULL).
		  * @param	state	Current player state.
		  */
		struct PlayerstateRead : public HandlerNotifyBase<void(const playerState_t* old, const playerState_t* state)> {};

		/**
		 * Called when a sound started to play/stopped.
		 *
		 * @param	sound	Sound structure containing various informations.
		 */
		struct Sound : public HandlerNotifyBase<void(const sound_t& sound)> {};

		/**
		 * Called when receiving the first valid snapshot on each map load. Useful to get the server time at this point.
		 *
		 * @param	snap	The first snapshot received.
		 */
		struct FirstSnapshot : public HandlerNotifyBase<void(const rawSnapshot_t& snap)> {};

		/**
		 * Called when the server has restarted (when a restart command has been issued on the server).
		 */
		struct ServerRestarted : public HandlerNotifyBase<void()> {};

		/**
		 * Called when a snap was received.
		 *
		 * @param	snap	The snapshot that was received.
		 */
		struct SnapReceived : public HandlerNotifyBase<void(const rawSnapshot_t& snap)> {};

		/**
		 * Called when a client is not visible to the player.
		 * It is used to update radar information (radar = teammate icons on the compass)
		 * to be able to keep track of players (teammates) that are not visible.
		 */
		struct ReadNonPVSClient : public HandlerNotifyBase<void(const radarUnpacked_t& radarUnpacked)> {};
	}

	class ServerGameState;
	class ClientTime;

	class ServerSnapshotManager
	{
	public:
		struct HandlerList
		{
		public:
			FunctionList<ClientHandlers::Sound> soundHandler;
			FunctionList<ClientHandlers::PlayerstateRead> playerStateReadHandler;
			FunctionList<ClientHandlers::FirstSnapshot> firstSnapshotHandler;
			FunctionList<ClientHandlers::SnapReceived> snapshotParsedHandler;
			FunctionList<ClientHandlers::ServerRestarted> serverRestartedHandler;
			FunctionList<ClientHandlers::ReadNonPVSClient> readNonPVSClientHandler;
		};

		ServerSnapshotManager(protocolType_c protocol);

		MOHPC_NET_EXPORTS HandlerList& getHandlers();
		MOHPC_NET_EXPORTS const HandlerList& getHandlers() const;

		/**
		 * Return snapshot data.
		 *
		 * @param	snapshotNum		The snapshot number to get data from.
		 * @param	outSnapshot		Output data.
		 * @return	true if the snap is valid.
		 */
		MOHPC_NET_EXPORTS bool getSnapshot(uintptr_t snapshotNum, SnapshotInfo& outSnapshot) const;

		/**
		 * Return snapshot data.
		 *
		 * @param	outSnapshot		Output data.
		 * @return	true if the snap is valid.
		 */
		MOHPC_NET_EXPORTS bool getSnapshot(SnapshotInfo& outSnapshot) const;

		/** Return the (raw) snapshot data. */
		MOHPC_NET_EXPORTS const rawSnapshot_t& getCurrentSnap() const;

		/** Return the current snapshot number. */
		MOHPC_NET_EXPORTS uint32_t getCurrentSnapNumber() const;
		MOHPC_NET_EXPORTS netTime_t getServerTime() const;
		MOHPC_NET_EXPORTS bool isSnapshotValid() const;
		MOHPC_NET_EXPORTS bool hasNewSnapshots() const;

		void setNewSnap(rawSnapshot_t& newSnap);

		void parseSnapshot(
			MSG& msg,
			const ServerGameState& clGameState,
			ClientTime& clientTime,
			ICommandSequence* serverCommands,
			tickTime_t currentTime,
			uint32_t serverMessageSequence,
			uint32_t outSequenceNum
		);

		bool checkTime(ClientTime& clientTime);

	private:
		void notifySounds(const rawSnapshot_t* newFrame);
		void serverRestarted();
		void updateSnapFlags();
		//void calculatePing(uint64_t currentTime, const OutgoingPackets& outPackets, uint32_t sequenceNum);
		bool firstSnapshot();
		bool isActive() const;

	private:
		const Parsing::IEntity* entityParser;
		const Parsing::IPlayerState* playerStateParser;
		const Parsing::IPVS* PVSParser;
		const Parsing::ISnapshot* snapshotParser;

	private:
		uint32_t parseEntitiesNum;
		uint32_t lastSnapFlags;
		HandlerList handlerList;
		rawSnapshot_t currentSnap;
		rawSnapshot_t snapshots[PACKET_BACKUP];
		entityState_t parseEntities[MAX_PARSE_ENTITIES];
		bool snapActive;
		bool newSnapshots;
	};
}

MOHPC_FUNCTIONLIST_TEMPLATE(MOHPC_NET_TEMPLATE, MOHPC_NET_EXPORTS, Network::ClientHandlers::PlayerstateRead);
MOHPC_FUNCTIONLIST_TEMPLATE(MOHPC_NET_TEMPLATE, MOHPC_NET_EXPORTS, Network::ClientHandlers::Sound);
MOHPC_FUNCTIONLIST_TEMPLATE(MOHPC_NET_TEMPLATE, MOHPC_NET_EXPORTS, Network::ClientHandlers::FirstSnapshot);
MOHPC_FUNCTIONLIST_TEMPLATE(MOHPC_NET_TEMPLATE, MOHPC_NET_EXPORTS, Network::ClientHandlers::ServerRestarted);
MOHPC_FUNCTIONLIST_TEMPLATE(MOHPC_NET_TEMPLATE, MOHPC_NET_EXPORTS, Network::ClientHandlers::SnapReceived);
MOHPC_FUNCTIONLIST_TEMPLATE(MOHPC_NET_TEMPLATE, MOHPC_NET_EXPORTS, Network::ClientHandlers::ReadNonPVSClient);
}