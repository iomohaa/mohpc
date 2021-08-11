#pragma once

#include "../NetGlobal.h"
#include "../ProtocolSingleton.h"
#include "../../Utility/Misc/MSG/Serializable.h"
#include "../Types/Protocol.h"

namespace MOHPC
{
namespace Network
{
	struct gameState_t;

	namespace Parsing
	{
		/*
		class IGameState : public ISerializableMessage
		{
		public:
			IGameState(const IClientGameProtocol& clientProto, IGameState& gameStateValue);

			size_t getCommandSequence() const;

		public:
			const IClientGameProtocol& clientProto;
			IGameState& gameState;

		protected:
			size_t commandSequence;
		};
		*/

		struct MOHPC_NET_EXPORTS gameStateClient_t
		{
		public:
			gameStateClient_t();

		public:
			uint32_t commandSequence;
			uint32_t clientNum;
			uint32_t checksumFeed;
		};

		struct MOHPC_NET_EXPORTS gameStateResults_t
		{
		public:
			gameStateResults_t();

		public:
			uint32_t serverDeltaTime;
			float serverDeltaTimeSeconds;
			uint32_t serverId;
			serverType_e serverType;
		};

		class MOHPC_NET_EXPORTS IGameState : public IProtocolSingleton<IGameState>
		{
		public:
			virtual gameState_t create() const = 0;
			virtual void parseGameState(MSG& msg, gameState_t& gameState, gameStateClient_t& results) const = 0;
			virtual void parseConfig(gameState_t& gameState, gameStateResults_t& results) const = 0;
			virtual serverType_e parseServerType(const char* version, size_t len) const = 0;
			virtual void saveGameState(MSG& msg, gameState_t& gameState, const gameStateClient_t& client) const = 0;
		};
	}
}
}