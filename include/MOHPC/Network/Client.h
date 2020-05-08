#pragma once

#include "../Global.h"
#include "../Script/Container.h"
#include "Event.h"
#include "Types.h"
#include "Channel.h"
#include "Encoding.h"
#include "InfoTypes.h"
#include "../Utilities/SharedPtr.h"
#include "../Utilities/RequestHandler.h"
#include "GamespyRequest.h"
#include "../Managers/NetworkManager.h"
#include "../Misc/MSG/MSG.h"
#include <stdint.h>
#include <functional>

namespace MOHPC
{
	class MSG;
	class IMessageStream;
	class TokenParser;
	class ReadOnlyInfo;

	namespace Network
	{
		static constexpr size_t PACKET_BACKUP = 32;
		static constexpr size_t PACKET_MASK = PACKET_BACKUP - 1;
		static constexpr size_t MAX_PARSE_ENTITIES = 2048;
		static constexpr size_t CMD_BACKUP = 128;
		static constexpr size_t CMD_MASK = CMD_BACKUP - 1;
		static constexpr size_t MAX_PACKET_USERCMDS = 32;

		class INetchan;

		using ClientGameConnectionPtr = SharedPtr<class ClientGameConnection>;

		class ClientSnapshot
		{
		public:
			static constexpr size_t MAX_MAP_AREA_BYTES = 32;
			static constexpr uintptr_t SNAPFLAG_RATE_DELAYED = 1;
			// snapshot used during connection and for zombies
			static constexpr uintptr_t SNAPFLAG_NOT_ACTIVE = 2;
			// toggled every map_restart so transitions can be detected
			static constexpr uintptr_t SNAPFLAG_SERVERCOUNT = 4;

		public:
			bool valid;
			uint8_t snapFlags;
			uint8_t serverTimeResidual;
			uint32_t serverTime;
			uint32_t messageNum;
			int32_t deltaNum;
			uint8_t areamask[MAX_MAP_AREA_BYTES];
			uint8_t cmdNum;
			playerState_t ps;

			uint32_t numEntities;
			uint32_t parseEntitiesNum;
			uint32_t serverCommandNum;

			uint32_t number_of_sounds;

		public:
			ClientSnapshot();
		};

		class IClient
		{
		private:
			INetchanPtr netchan;

		public:
			IClient(const INetchanPtr& inNetchan)
				: netchan(inNetchan)
			{}

			virtual ~IClient() = default;

			/** Handler for receiving message. */
			virtual void receive(const netadr_t& from, MSG& msg) = 0;

			/** Tick function for ticking. */
			virtual void tick() {};

			/** Send a client command. */
			virtual void sendCommand(const char* cmd) = 0;

			INetchan* getNetchan() const;
		};

		using IClientPtr = SharedPtr<IClient>;

		/** Class for handling client specifing settings. */
		class MOHPC_EXPORTS ClientInfo
		{
		private:
			str name;
			str playerAlliedModel;
			str playerGermanModel;

		public:
			ClientInfo();
			ClientInfo(ClientInfo&& other) = default;
			ClientInfo& operator=(ClientInfo&& other) = default;

			void setName(const char* newName);
			const char* getName() const;

			void setPlayerAlliedModel(const char* newModel);
			const char* getPlayerAlliedModel() const;

			void setPlayerGermanModel(const char* newModel);
			const char* getPlayerGermanModel() const;
		};

		class ClientInstance : public ITickableNetwork
		{
		private:
			IClientPtr client;

		public:
			MOHPC_EXPORTS ClientInstance(const IClientPtr& inClient);
			MOHPC_EXPORTS ~ClientInstance();

			MOHPC_EXPORTS void processEvents();
			MOHPC_EXPORTS virtual void tick(uint64_t deltaTime, uint64_t currentTime) override;
			MOHPC_EXPORTS void setClient(const IClientPtr& newClient);
			MOHPC_EXPORTS IClientPtr getClient() const;
		};
	}
}
