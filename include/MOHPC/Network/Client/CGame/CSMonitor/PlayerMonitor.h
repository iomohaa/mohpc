#pragma once

#include "ConfigStringMonitor.h"
#include "../ClientInfo.h"
#include "../../../../Utility/SharedPtr.h"

namespace MOHPC
{
namespace Network
{
namespace CGame
{
	namespace Handlers
	{
		/**
		 * A weapon resource has been added, modified or removed.
		 *
		 * @param index The index to resource.
		 * @param name Weapon name, can be a class name or a weapon name.
		 */
		class WeaponModified : public HandlerNotifyBase<void(size_t index, const char* name)> {};

		/**
		 * The number of teams was modified.
		 *
		 * @param count The number of teams that can be chosen in game.
		 */
		class TeamsModified : public HandlerNotifyBase<void(uint32_t count)> {};

		/**
		 * Called when generic strings was modified.
		 *
		 * @param value New generic strings value.
		 */
		class StringsModified : public HandlerNotifyBase<void(const char* value)> {};

		/**
		 * The number of players in a team was modified.
		 *
		 * @param team The team that was modified.
		 * @param index Index of the team in the server.
		 * @param count The number of players.
		 */
		class TeamCountModified : public HandlerNotifyBase<void(teamType_e team, uint32_t index, size_t count)> {};
	}

	class PlayerMonitor : public ConfigstringMonitor
	{
		struct HandlerList
		{
			FunctionList<Handlers::WeaponModified> weaponModifiedHandler;
			FunctionList<Handlers::TeamsModified> teamsModifiedHandler;
			FunctionList<Handlers::StringsModified> stringsModifiedHandler;
			FunctionList<Handlers::TeamCountModified> teamCountModifiedHandler;
		};

		MOHPC_NET_OBJECT_DECLARATION(PlayerMonitor);

	public:
		MOHPC_NET_EXPORTS PlayerMonitor(const SnapshotProcessorPtr& snapshotProcessor, const ClientInfoListPtr& clientListPtr);
		MOHPC_NET_EXPORTS ~PlayerMonitor();

		HandlerList& getHandlerList();

		bool configStringModified(csNum_t num, const char* cs) override;

	private:
		void parseTeam(const char* data, uint32_t& type, uint32_t& count);
		void weaponModified(size_t index, const char* cs);
		void teamsModified(const char* cs);
		void stringsModified(const char* cs);
		void spectatorsModified(const char* cs);
		void alliesModified(const char* cs);
		void axisModified(const char* cs);

	private:
		ClientInfoListPtr clientList;
		HandlerList handlerList;
	};
	using PlayerMonitorPtr = SharedPtr<PlayerMonitor>;
}
}
}
