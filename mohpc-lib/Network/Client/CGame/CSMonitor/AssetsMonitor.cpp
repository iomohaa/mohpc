#include <MOHPC/Network/Client/CGame/CSMonitor/AssetsMonitor.h>
#include <MOHPC/Network/Client/GameState.h>

using namespace MOHPC;
using namespace MOHPC::Network;
using namespace MOHPC::Network::CGame;

MOHPC_OBJECT_DEFINITION(AssetsMonitor);

AssetsMonitor::AssetsMonitor(const SnapshotProcessorPtr& snapshotProcessor)
	: ConfigstringMonitor(snapshotProcessor)
{
}

AssetsMonitor::~AssetsMonitor()
{
}

bool AssetsMonitor::configStringModified(csNum_t num, const char* cs)
{
	if (num >= CS::MODELS && num < CS::MODELS + CS::MAX_MODELS)
	{
		modelModified(num - CS::MODELS, cs);

		return true;
	}
	else if (num >= CS::SOUNDS && num < CS::SOUNDS + CS::MAX_SOUNDS)
	{
		soundModified(num - CS::SOUNDS, cs);

		return true;
	}
	else if (num >= CS::IMAGES && num < CS::IMAGES + CS::MAX_IMAGES)
	{
		imageModified(num - CS::IMAGES, cs);

		return true;
	}
	else if (num >= CS::LIGHTSTYLES && num < CS::LIGHTSTYLES + CS::MAX_LIGHTSTYLES)
	{
		lightStyleModified(num - CS::LIGHTSTYLES, cs);

		return true;
	}

	return false;
}

AssetsMonitor::HandlerList& AssetsMonitor::getHandlerList()
{
	return handlerList;
}

void AssetsMonitor::modelModified(size_t index, const char* value)
{
	handlerList.modelHandler.broadcast(value);
}

void AssetsMonitor::soundModified(size_t index, const char* value)
{
	handlerList.soundHandler.broadcast(value);
}

void AssetsMonitor::imageModified(size_t index, const char* value)
{
	handlerList.imageHandler.broadcast(value);
}

void AssetsMonitor::lightStyleModified(size_t index, const char* value)
{
	handlerList.lightStyleHandler.broadcast(value);
}
