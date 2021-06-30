#pragma once

#include "../../../Utility/HandlerList.h"
#include "../../../Utility/Function.h"
#include "../../Types/Reliable.h"
#include "../../Types/Snapshot.h"
#include "../../Exception.h"

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
		}

		/**
		 * Contains states for the current and the next frame.
		 */
		struct EntityInfo
		{
		public:
			entityState_t currentState;
			entityState_t nextState;
			uint32_t snapshotTime;
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
			MOHPC_NET_EXPORTS uint32_t getSnapshotTime() const;
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
		public:
			struct HandlerList
			{
				FunctionList<Handlers::EntityAdded> entityAddedHandler;
				FunctionList<Handlers::EntityRemoved> entityRemovedHandler;
				FunctionList<Handlers::EntityModified> entityModifiedHandler;
			};

		public:
			SnapshotProcessor(CommandManager& serverCommandManagerRef);

			void init(uintptr_t serverMessageSequence, rsequence_t serverCommandSequence);

			MOHPC_NET_EXPORTS const HandlerList& handlers() const;
			MOHPC_NET_EXPORTS HandlerList& handlers();

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
			void processSnapshots(uint64_t serverTime);
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

		public:
			HandlerList handlerList;
			CommandManager& serverCommandManager;
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
			bool nextFrameTeleport : 1;
			bool nextFrameCameraCut : 1;
			bool thisFrameTeleport : 1;
		};

		namespace CGSnapshotError
		{
			class Base : public NetworkException {};

			/**
			 * The next snap time is backward of the client time.
			 */
			class NextSnapTimeWentBackward : public Base
			{
			public:
				NextSnapTimeWentBackward(uint64_t inPrevTime, uint64_t inTime);

				/** Return the client time. */
				MOHPC_NET_EXPORTS uint64_t getClientTime() const;
				/** Return the time of the snap. */
				MOHPC_NET_EXPORTS uint64_t getSnapTime() const;

			private:
				uint64_t oldTime;
				uint64_t time;
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