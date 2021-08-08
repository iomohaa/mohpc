#pragma once

#include "../NetGlobal.h"
#include "../NetObject.h"
#include "../../Utility/HandlerList.h"

#include "InputModuleInterface.h"
#include "UserInput.h"

#include <cstdint>

namespace MOHPC
{
class MSG;

namespace Network
{
	class protocolType_c;

	namespace Parsing
	{
		class IUserMove;
	}

	namespace ClientHandlers
	{
		/**
		 * This callback is used to modify the player input before sending.
		 *
		 * @param	cmd		User command structure.
		 * @param	eyes	Eyes information.
		 */
		struct UserInput : public HandlerNotifyBase<void(usercmd_t& cmd, usereyes_t& eyes)> {};
	}

	class UserGameInputModule : public IUserInputModule
	{
		MOHPC_NET_OBJECT_DECLARATION(UserGameInputModule);

	public:
		struct HandlerList
		{
		public:
			FunctionList<ClientHandlers::UserInput> userInputHandler;
		};

	public:
		MOHPC_NET_EXPORTS UserGameInputModule(const protocolType_c& protocolType);
		MOHPC_NET_EXPORTS ~UserGameInputModule();

		MOHPC_NET_EXPORTS const UserInput& getUserInput() const;
		MOHPC_NET_EXPORTS UserInput& getUserInput();
		MOHPC_NET_EXPORTS HandlerList& getHandlerList();

		void process(const ClientTime& time) override;
		bool canWrite() const override;
		void write(MSG& msg, uint32_t key, bool deltaMove);

	private:
		uint8_t getNumCommandsToWrite(uint32_t index);

	private:
		const Parsing::IUserMove* userMoveWriter;
		HandlerList handlerList;
		UserInput input;
		usereyes_t oldeyes;
		uint32_t lastCmdNum;
	};
	using UserGameInputModulePtr = SharedPtr<UserGameInputModule>;
}
}