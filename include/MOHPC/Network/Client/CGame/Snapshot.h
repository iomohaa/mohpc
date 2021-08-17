#pragma once

#include "../../NetGlobal.h"
#include "../../NetObject.h"
#include "../../../Utility/HandlerList.h"
#include "../../../Utility/Function.h"
#include "../../../Utility/CommandManager.h"
#include "../../Types/Reliable.h"
#include "../../Types/Snapshot.h"
#include "../../Exception.h"
#include "../../Configstring.h"
#include "../GameState.h"

#include <functional>
#include <cstdint>
#include <cstddef>

namespace MOHPC
{
class CommandManager;
class TokenParser;

namespace Network
{
	class entityState_t;
	class ServerSnapshotManager;
	class ClientTime;
	class SnapshotInfo;

	static constexpr size_t MAX_ACTIVE_SNAPSHOTS = 2;

	namespace CGame
	{
		namespace Handlers
		{
			/**
			 * Called when an entity has just been added/spawned.
			 *
			 * @param	entity	The entity that was added.
			 */
			struct EntityAdded : public HandlerNotifyBase<void(const entityState_t& state)> {};

			/**
			 * Called when an entity has just been removed.
			 *
			 * @param	entity	The entity that was removed.
			 */
			struct EntityRemoved : public HandlerNotifyBase<void(const entityState_t& state)> {};

			/**
			 * Called when an entity has just been modified.
			 *
			 * @param	entity	The entity that was modified.
			 */
			struct EntityModified : public HandlerNotifyBase<void(const entityState_t& prev, const entityState_t& current)> {};

			/**
			 * Called after the insertion/modification of a new configstring.
			 *
			 * @param	csNum			The configstring num.
			 * @param	configString	The string pointed at by the csNum.
			 */
			struct Configstring : public HandlerNotifyBase<void(csNum_t csNum, const char* configString)> {};

			/** This is called after all configstrings have been parsed. */
			struct ProcessedConfigstrings : public HandlerNotifyBase<void()> {};
		}

		/**
		 * Contains states for the current and the next frame.
		 */
		struct EntityInfo
		{
		public:
			entityState_t currentState;
			entityState_t nextState;
			netTime_t snapshotTime;
			bool currentValid : 1;
			bool interpolate : 1;
			bool teleported : 1;

		public:
			EntityInfo();

			/** Get the current entity state. */
			MOHPC_NET_EXPORTS const entityState_t& getCurrentState() const;
			/** Get the next entity state from the next incoming snap. */
			MOHPC_NET_EXPORTS const entityState_t& getNextState() const;
			/** Get the last updated time (snapshot time). */
			MOHPC_NET_EXPORTS netTime_t getSnapshotTime() const;
			/** Whether or not this entity info exists in world. */
			MOHPC_NET_EXPORTS bool isValid() const;
			/** Whether or not this entity info should interpolate. */
			MOHPC_NET_EXPORTS bool isInterpolating() const;
			MOHPC_NET_EXPORTS bool hasTeleported() const;
		};

		/**
		 * The snapshot processor handles parsing snapshot and entities in snapshots.
		 * Also processes server commands in the active snapshot.
		 */
		class SnapshotProcessor
		{
			MOHPC_NET_OBJECT_DECLARATION(SnapshotProcessor);

		public:
			struct HandlerList
			{
				FunctionList<Handlers::EntityAdded> entityAddedHandler;
				FunctionList<Handlers::EntityRemoved> entityRemovedHandler;
				FunctionList<Handlers::EntityModified> entityModifiedHandler;
				FunctionList<Handlers::Configstring> configstringModifiedHandler;
				FunctionList<Handlers::ProcessedConfigstrings> processedConfigStringsHandler;
			};

		public:
			SnapshotProcessor();
			~SnapshotProcessor();

			void init(const ServerGameStatePtr& gs, uintptr_t serverMessageSequence, rsequence_t serverCommandSequence);

			MOHPC_NET_EXPORTS const HandlerList& handlers() const;
			MOHPC_NET_EXPORTS HandlerList& handlers();

			MOHPC_NET_EXPORTS CommandManager& getCommandManager();
			MOHPC_NET_EXPORTS const CommandManager& getCommandManager() const;

			/** Return the client time. */
			MOHPC_NET_EXPORTS const ClientTime& getClientTime() const;

			/** Return the previous snap. */
			MOHPC_NET_EXPORTS const SnapshotInfo& getOldSnap() const;
			/** Return the current active snap. Can be null. */
			MOHPC_NET_EXPORTS const SnapshotInfo* getSnap() const;
			/** Return the next snap. Can be null. */
			MOHPC_NET_EXPORTS const SnapshotInfo* getNextSnap() const;

			/** Get the entity with the specified number. */
			MOHPC_NET_EXPORTS const EntityInfo* getEntity(entityNum_t num) const;

			/** Return true if no lerp/interpolation should be done next frame (instant teleport). */
			MOHPC_NET_EXPORTS bool doesTeleportNextFrame() const;
			/** Return true if the active camera cuts at the next frame. */
			MOHPC_NET_EXPORTS bool doesCameraCutNextFrame() const;
			/** Return true if no lerp/interpolation should be done (instant teleport). */
			MOHPC_NET_EXPORTS bool doesTeleportThisFrame() const;
			/** Set the teleport this frame flag. */
			MOHPC_NET_EXPORTS void makeTeleportThisFrame();
			/** Clear the teleport this frame flag. */
			MOHPC_NET_EXPORTS void clearTeleportThisFrame();

			/**
			 * Snapshot parsing
			 */
			SnapshotInfo* readNextSnapshot();
			void processSnapshots(tickTime_t simulatedRemoteTime);
			void setNextSnap(SnapshotInfo* newSnap);
			void setInitialSnapshot(SnapshotInfo* newSnap);
			void executeNewServerCommands(uintptr_t serverCommandSequence, bool differentServer);
			//====

			/**
			 * Entity transitioning
			 */
			void transitionSnapshot(bool differentServer);
			void transitionEntity(EntityInfo& entInfo);
			void buildSolidList();
			//====

			void setPtrs(
				const ClientTime* clientTimePtr,
				const ServerSnapshotManager* snapshotManagerPtr,
				const ICommandSequence* commandSequencePtr
			);

		private:
			void configStringModified(TokenParser& tokenized);

		public:
			CommandTemplate<SnapshotProcessor, &SnapshotProcessor::configStringModified> csModified;

		public:
			HandlerList handlerList;
			CommandManager serverCommandManager;
			const ClientTime* clientTime;
			const ServerSnapshotManager* snapshotManager;
			const ICommandSequence* commandSequence;
			SnapshotInfo* nextSnap;
			SnapshotInfo* snap;
			SnapshotInfo oldSnap;
			SnapshotInfo activeSnapshots[MAX_ACTIVE_SNAPSHOTS];
			EntityInfo clientEnts[MAX_GENTITIES];
			uintptr_t processedSnapshotNum;
			uintptr_t latestSnapshotNum;
			rsequence_t latestCommandSequence;
			bool newConfigstrings : 1;
			bool nextFrameTeleport : 1;
			bool nextFrameCameraCut : 1;
			bool thisFrameTeleport : 1;
		};
		using ConstSnapshotProcessorPtr = SharedPtr<const SnapshotProcessor>;
		using SnapshotProcessorPtr = SharedPtr<SnapshotProcessor>;

		namespace CGSnapshotError
		{
			class Base : public NetworkException {};

			/**
			 * The next snap time is backward of the client time.
			 */
			class NextSnapTimeWentBackward : public Base
			{
			public:
				NextSnapTimeWentBackward(netTime_t inPrevTime, netTime_t inTime);

				/** Return the client time. */
				MOHPC_NET_EXPORTS netTime_t getClientTime() const;
				/** Return the time of the snap. */
				MOHPC_NET_EXPORTS netTime_t getSnapTime() const;

			private:
				netTime_t oldTime;
				netTime_t time;
			};

			/**
			 * The next snapshot number is lower than the latest snapshot number.
			 */
			class SnapNumWentBackward : public Base
			{
			public:
				SnapNumWentBackward(uintptr_t newNum, uintptr_t latestNum);

				/** Return the new snapshot number. */
				MOHPC_NET_EXPORTS uintptr_t getNewNum() const;
				/** Return the latest snapshot number. */
				MOHPC_NET_EXPORTS uintptr_t getLatestNum() const;

			private:
				uintptr_t newNum;
				uintptr_t latestNum;
			};

			/**
			 * Null snapshot when processing snapshots.
			 */
			class NullSnapshot : public Base {};
		}
	}
}
}