#include "PlayingState.h"
#include "System.h"
#include "Game.h"

PlayingState::PlayingState()
{
}

PlayingState::~PlayingState()
{
}

void PlayingState::OnActivate(System *system, StateArgumentMap &args)
{
	Game *game = system->GetGame();

	level_ = args["Level"].asInt;
	game->InitialiseLevel(level_);
}

void PlayingState::OnUpdate(System *system)
{
	Game *game = system->GetGame();
	game->Update(system);

	GameState::StateArgument arg;
	arg.asInt = game->GetScore();

	GameState::StateArgumentMap argMap;
	argMap.insert(std::make_pair(std::string("CurrentScore"), arg));

	if (game->IsGameOver())
	{
		game->ResetGame();
		system->SetNextState(std::string("GameOver"), argMap);
	}
	else if (game->IsLevelComplete())
	{
		StateArgumentMap args;
		args["Level"].asInt = level_ + 1;
		system->SetNextState("LevelStart", args);
	}
}

void PlayingState::OnRender(System *system)
{
	Game *game = system->GetGame();
	game->RenderEverything(system->GetGraphics());
}

void PlayingState::OnDeactivate(System *system)
{
}
