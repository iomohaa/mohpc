#include <MOHPC/Network/Types/GameState.h>

using namespace MOHPC;
using namespace MOHPC::Network;

EntityList::EntityList()
	: entityList(nullptr)
	, maxEntities(0)
{
}

EntityList::EntityList(entityNum_t maxEntitiesValue)
	: maxEntities(maxEntitiesValue)
{
	if (maxEntities)
	{
		// assign an array of EntityList
		entityList = new entityState_t[maxEntities];
	}
	else
	{
		// no entities
		entityList = nullptr;
	}
}

EntityList::EntityList(EntityList&& other)
	: entityList(other.entityList)
	, maxEntities(other.maxEntities)
{
	other.entityList = nullptr;
	other.maxEntities = 0;
}

EntityList& EntityList::operator=(EntityList&& other)
{
	freeList();

	entityList = other.entityList;
	maxEntities = other.maxEntities;
	other.entityList = nullptr;
	other.maxEntities = 0;

	return *this;
}

EntityList::~EntityList()
{
	freeList();
}

void EntityList::freeList()
{
	if (entityList)
	{
		delete[] entityList;
	}
}

const entityState_t& EntityList::getEntity(entityNum_t index) const
{
	if (index >= getMaxEntities()) {
		throw GameStateErrors::EntityOutOfRangeException(index);
	}

	return entityList[index];
}

entityState_t& EntityList::getEntity(entityNum_t index)
{
	if (index >= getMaxEntities()) {
		throw GameStateErrors::EntityOutOfRangeException(index);
	}

	return entityList[index];
}

entityNum_t EntityList::getMaxEntities() const
{
	return maxEntities;
}

void EntityList::reset()
{
	for (size_t i = 0; i < maxEntities; ++i)
	{
		entityList[i] = entityState_t();
	}
}

mapInfo_t::mapInfo_t()
	: serverId(0)
	, checksumFeed(0)
{
}

mapInfo_t::mapInfo_t(uint32_t serverIdVal, uint32_t checksumFeedVal)
	: serverId(serverIdVal)
	, checksumFeed(checksumFeedVal)
{
}

uint32_t mapInfo_t::getServerId() const
{
	return serverId;
}

uint32_t mapInfo_t::getChecksumFeed() const
{
	return checksumFeed;
}

mapInfo_t& gameState_t::getMapInfo()
{
	return mapInfo;
}

const mapInfo_t& gameState_t::getMapInfo() const
{
	return mapInfo;
}


gameState_t::gameState_t()
{
}

gameState_t::gameState_t(const size_t numConfigStrings, const size_t maxChars, const entityNum_t maxBaselines)
	: csMan(numConfigStrings, maxChars)
	, entityBaselines(maxBaselines)
{
}

gameState_t::gameState_t(gameState_t&& other)
	: csMan(std::move(other.csMan))
	, entityBaselines(std::move(other.entityBaselines))
{
}

gameState_t& gameState_t::operator=(gameState_t&& other)
{
	csMan = std::move(other.csMan);
	entityBaselines = std::move(other.entityBaselines);
	return *this;
}

gameState_t::~gameState_t()
{
}

ConfigStringManager& gameState_t::getConfigstringManager()
{
	return csMan;
}

const ConfigStringManager& gameState_t::getConfigstringManager() const
{
	return csMan;
}

EntityList& gameState_t::getEntityBaselines()
{
	return entityBaselines;
}

const EntityList& gameState_t::getEntityBaselines() const
{
	return entityBaselines;
}

void gameState_t::reset()
{
	csMan.reset();
	entityBaselines.reset();
	mapInfo = mapInfo_t();
}

GameStateErrors::EntityOutOfRangeException::EntityOutOfRangeException(entityNum_t inEntityNum)
	: entityNum(inEntityNum)
{}

entityNum_t GameStateErrors::EntityOutOfRangeException::getEntityNum() const
{
	return entityNum;
}

str GameStateErrors::EntityOutOfRangeException::what() const
{
	return std::to_string(getEntityNum());
}
