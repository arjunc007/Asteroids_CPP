#include <string>

#include "PlayingState.h"
#include "System.h"
#include "Graphics.h"
#include "Game.h"

PlayingState::PlayingState() : scoreFont_(0)
{
}

PlayingState::~PlayingState()
{
}

void PlayingState::OnActivate(System *system, StateArgumentMap &args)
{
	if(scoreFont_ == 0)
	{
		scoreFont_ = system->GetGraphics()->CreateXFont("Arial", 48);
		popupFont_ = system->GetGraphics()->CreateXFont("Arial", 16);
	}

	Game *game = system->GetGame();

	level_ = args["Level"].asInt;
	game->InitialiseLevel(level_);
}

void PlayingState::OnUpdate(System *system)
{
	Game *game = system->GetGame();
	game->Update(system);
	if (game->IsGameOver())
	{
		game->ResetGame();
		system->SetNextState("GameOver");
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
	scoreFont_->DrawText("Score: " + std::to_string(game->GetScore()), 50, 500, 0xffffff00);

	std::list<Game::Score> popupList = game->GetPopups();
	for(auto popup : popupList)
		popupFont_->DrawText("+" + std::to_string(popup.value), popup.pos.x, popup.pos.y, popup.color);
}

void PlayingState::OnDeactivate(System *system)
{
	system->GetGraphics()->DestroyXFont(scoreFont_);
	system->GetGraphics()->DestroyXFont(popupFont_);
	scoreFont_ = 0;
	popupFont_ = 0;
}
