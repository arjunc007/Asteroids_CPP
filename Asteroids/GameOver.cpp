#include "GameOver.h"
#include "System.h"
#include "Graphics.h"
#include "FontEngine.h"
#include "Game.h"

GameOver::GameOver() :
	delay_(0)
{
	scoreBoard_ = std::make_unique<ScoreBoard>();
}

void GameOver::OnActivate(System *system, StateArgumentMap &args)
{
	delay_ = 1000;
	scoreBoard_->AddScore("", args["CurrentScore"].asInt);
}

void GameOver::OnUpdate(System *system)
{
	if (--delay_ == 0)
	{
		system->SetNextState("MainMenu");
	}
}

void GameOver::OnRender(System *system)
{
	Graphics *graphics = system->GetGraphics();
	FontEngine *fontEngine = graphics->GetFontEngine();

	system->GetGame()->RenderBackgroundOnly(graphics);

	const char *gameOverText = "Game Over";
	int textWidth = fontEngine->CalculateTextWidth(gameOverText, FontEngine::FONT_TYPE_LARGE);
	int textX = (800 - textWidth) / 2;
	int textY = (600 - 248) / 2;
	fontEngine->DrawText(gameOverText, textX, textY, 0xff00ffff, FontEngine::FONT_TYPE_LARGE);

	const std::vector<std::pair<std::string, int>>* highScores = scoreBoard_->GetHighScores();

	textY += 60;
	fontEngine->DrawText("High Scores", textX, textY, 0xff0000ff, FontEngine::FONT_TYPE_LARGE);
	textY += 48;
	for (auto score = highScores->begin(); score != highScores->end(); score++)
	{
		textY += 36;
		textWidth = fontEngine->CalculateTextWidth(std::to_string(score->second), FontEngine::FONT_TYPE_MEDIUM);
		textX = (800 - textWidth) / 2;
		fontEngine->DrawText(std::to_string(score->second), textX, textY, 0xffff00fff, FontEngine::FONT_TYPE_MEDIUM);
	}
}

void GameOver::OnDeactivate(System *system)
{
}
