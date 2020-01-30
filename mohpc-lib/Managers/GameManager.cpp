#include <Shared.h>
#include <MOHPC/Managers/AssetManager.h>
#include <MOHPC/Managers/GameManager.h>
#include <MOHPC/Script/Game.h>
#include <MOHPC/Script/Level.h>
#include <MOHPC/Script/World.h>

using namespace MOHPC;

CLASS_DEFINITION(GameManager)
GameManager::GameManager()
{
	world = nullptr;
}

void GameManager::Init()
{
}

World* GameManager::GetWorld() const
{
	return world;
}

Level* GameManager::GetLevel()
{
	return &level;
}

Game* GameManager::GetGame()
{
	return &game;
}
