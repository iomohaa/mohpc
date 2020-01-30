#include <Shared.h>
#include <MOHPC/Script/Level.h>

using namespace MOHPC;

Level::Level()
{
	framenum = 0;
	inttime = 0;
	intframetime = 0;
	time = 0.f;
	frametime = 0.f;
}

void Level::setTime(int levelTime)
{
	inttime = levelTime;
	time = inttime / 1000.0f;
}

void Level::setFrametime(int frametime)
{
	intframetime = frametime;
	this->frametime = frametime / 1000.0f;
}

const str& Level::GetMapName() const
{
	return current_map;
}

void Level::AddSimpleArchivedEntity(SimpleArchivedEntity* Entity)
{
	m_SimpleArchivedEntities.AddObject(Entity);
}

void Level::RemoveSimpleArchivedEntity(SimpleArchivedEntity* Entity)
{
	m_SimpleArchivedEntities.RemoveObject(Entity);
}

size_t Level::GetNumSimpleArchivedEntities() const
{
	return m_SimpleArchivedEntities.NumObjects();
}

SimpleArchivedEntity* Level::GetSimpleArchivedEntity(uintptr_t num) const
{
	return m_SimpleArchivedEntities.ObjectAt(num);
}

float Level::GetFrameTime() const
{
	return frametime;
}

float Level::GetTimeSeconds() const
{
	return time;
}

CLASS_DECLARATION(Listener, Level, NULL)
{
	{ NULL, NULL }
};
