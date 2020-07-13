#pragma once

#include "../Global.h"
#include "../Managers/Manager.h"
#include "../Script/Level.h"
#include "../Script/Game.h"
#include "../Script/World.h"

namespace MOHPC
{
	class Game;
	class Level;
	class World;

	class GameManager : public Manager
	{
		friend class World;

		CLASS_BODY(GameManager);

	private:
		Game game;
		Level level;
		World* world;

	public:
		MOHPC_EXPORTS GameManager();

		MOHPC_EXPORTS World* GetWorld() const;
		MOHPC_EXPORTS Level* GetLevel();
		MOHPC_EXPORTS Game* GetGame();

	protected:
		virtual void Init();
	};
	using GameManagerPtr = SharedPtr<GameManager>;
};
