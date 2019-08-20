#pragma once
#include "ship.h"
class UFO :
	public Ship
{
public:
	UFO(int speed);
	~UFO(void);

	void Update(System* system);
	void Render(Graphics* graphics) const;

	void Reset();

private:
	float speed_;
	std::clock_t startFrameTime_;
};

