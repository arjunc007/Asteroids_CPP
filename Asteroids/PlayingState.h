#ifndef PLAYINGSTATE_H_INCLUDED
#define PLAYINGSTATE_H_INCLUDED

#include "GameState.h"
#include "Font.h"

class PlayingState : public GameState
{
public:
	PlayingState();
	~PlayingState();

	void OnActivate(System *system, StateArgumentMap &args);
	void OnUpdate(System *system);
	void OnRender(System *system);
	void OnDeactivate(System *system);

private:

	int level_;
	Font* scoreFont_;
	Font* popupFont_;

};

#endif // PLAYINGSTATE_H_INCLUDED
