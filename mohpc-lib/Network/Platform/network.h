#include <MOHPC/Network/Remote/Socket.h>

namespace MOHPC
{
	namespace Platform
	{
		class Network
		{
		public:
			static MOHPC::Network::ISocketFactoryPtr createSockFactory();
		};
	}
}