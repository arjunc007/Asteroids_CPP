#ifndef GAMEOVER_H_INCLUDE
#define GAMEOVER_H_INCLUDE

#include "GameState.h"
#include "ScoreBoard.h"
#include <memory>

class GameOver : public GameState
{
public:
	GameOver();

	void OnActivate(System *system, StateArgumentMap &args);
	void OnUpdate(System *system);
	void OnRender(System *system);
	void OnDeactivate(System *system);

private:
	int delay_;
	std::unique_ptr<ScoreBoard> scoreBoard_;
};

#endif // GAMEOVER_H_INCLUDE
