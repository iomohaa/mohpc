#pragma once

#include "Entity.h"
#include "PlayerState.h"

#include <cstdint>

namespace MOHPC
{
namespace Network
{
	static constexpr size_t MAX_MAP_AREA_BYTES = 32;
	static constexpr size_t MAX_ENTITIES_IN_SNAPSHOT = 1024;
	static constexpr size_t MAX_SERVER_SOUNDS = 32;
	static constexpr size_t MAX_SERVER_SOUNDS_BITS = 6;

	static constexpr uintptr_t SNAPFLAG_RATE_DELAYED = 1;
	// snapshot used during connection and for zombies
	static constexpr uintptr_t SNAPFLAG_NOT_ACTIVE = 2;
	// toggled every map_restart so transitions can be detected
	static constexpr uintptr_t SNAPFLAG_SERVERCOUNT = 4;

	struct sound_t
	{
	public:
		const entityState_t* entity;
		const char* soundName;
		Vector origin;
		float volume;
		float minDist;
		float maxDist;
		float pitch;
		uint8_t channel;
		bool hasStopped : 1;
		bool isStreamed : 1;
		bool isSpatialized : 1;
		bool hasVolume : 1;
		bool hasDist : 1;
		bool hasPitch : 1;

	public:
		sound_t();

		/** The entity that the sound is playing on. */
		MOHPC_NET_EXPORTS const entityState_t* getEntityState() const;
		/** The name of the sound (retrieved from configstrings). */
		MOHPC_NET_EXPORTS const char* getName() const;
		/** The sound origin if it's spatialized. */
		MOHPC_NET_EXPORTS const Vector& getOrigin() const;

		/** Sound's volume. */
		MOHPC_NET_EXPORTS float getVolume() const;
		/** The minimum distance the sound will play at full volume. */
		MOHPC_NET_EXPORTS float getMinimumDistance() const;
		/** The maximum distance before the sound stops playing. */
		MOHPC_NET_EXPORTS float getMaximumDistance() const;
		/** The pitch of the sound. */
		MOHPC_NET_EXPORTS float getPitch();
		/** The channel the sound is playing on. */
		MOHPC_NET_EXPORTS uint8_t getChannel() const;

		/** Whether or not the sound has stopped playing. */
		MOHPC_NET_EXPORTS bool hasSoundStopped() const;
		/** If the whole sound is not into memory. */
		MOHPC_NET_EXPORTS bool isStreamedSound() const;
		/** If the sound is spatialized (check origin).*/
		MOHPC_NET_EXPORTS bool isSpatializedSound() const;

		/** Volume is assigned if yes. */
		MOHPC_NET_EXPORTS bool hasSoundVolume() const;
		/** minDist is assigned if yes. */
		MOHPC_NET_EXPORTS bool hasSoundDistance() const;
		/** pitch is assigned if yes. */
		MOHPC_NET_EXPORTS bool hasSoundPitch() const;
	};

	struct rawSnapshot_t
	{
	public:
		int32_t deltaNum;
		uint32_t ping;
		uint32_t serverTime;
		uint32_t messageNum;
		uint32_t numEntities;
		uint32_t parseEntitiesNum;
		uint32_t serverCommandNum;
		uint32_t numSounds;
		uint8_t snapFlags;
		uint8_t serverTimeResidual;
		uint8_t areamask[MAX_MAP_AREA_BYTES];
		uint8_t cmdNum;
		playerState_t ps;
		bool valid;
		sound_t sounds[MAX_SERVER_SOUNDS];

	public:
		rawSnapshot_t();
	};

	class SnapshotInfo
	{
	public:
		SnapshotInfo();

		/** Flags of the snap (see SNAPFLAG_* values). */
		MOHPC_NET_EXPORTS uint32_t getSnapFlags() const;
		/** Ping of the client in snap. */
		MOHPC_NET_EXPORTS uint32_t getPing() const;

		/** Server time when the server sent the snap. */
		MOHPC_NET_EXPORTS uint32_t getServerTime() const;

		/** Area mask at index (for visibility). */
		MOHPC_NET_EXPORTS uint8_t getAreaMask(uint8_t index) const;

		/** Complete information about the current player at this time. */
		MOHPC_NET_EXPORTS const playerState_t& getPlayerState() const;

		/** Number of entities that need to be presented in this snap. */
		MOHPC_NET_EXPORTS size_t getNumEntities() const;
		/** Entity at the specified index. */
		MOHPC_NET_EXPORTS const entityState_t& getEntityState(entityNum_t index) const;
		/** Search for an entity by a global entity number. */
		MOHPC_NET_EXPORTS const entityState_t* getEntityStateByNumber(entityNum_t entityNum) const;

		/** Number of server commands that was received in this snap. */
		MOHPC_NET_EXPORTS size_t getNumServerCommands();
		/** Start sequence number of the command in list. */
		MOHPC_NET_EXPORTS size_t getServerCommandSequence() const;

		/** Number of sounds in this snap*/
		MOHPC_NET_EXPORTS size_t getNumSounds() const;
		/** Sound at the specified index. */
		MOHPC_NET_EXPORTS const sound_t& getSound(uint8_t index) const;

	public:
		size_t numEntities;
		size_t numSounds;
		size_t numServerCommands;
		size_t serverCommandSequence;
		uint32_t snapFlags;
		uint32_t ping;
		uint32_t serverTime;
		playerState_t ps;
		entityState_t entities[MAX_ENTITIES_IN_SNAPSHOT];
		sound_t sounds[MAX_SERVER_SOUNDS];
		uint8_t areamask[MAX_MAP_AREA_BYTES];
	};
}
}
