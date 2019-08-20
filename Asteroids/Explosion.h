#ifndef EXPLOSION_H_INCLUDED
#define EXPLOSION_H_INCLUDED

#include <list>
#include <ctime>
#include "GameEntity.h"

struct Particle
{
	XMFLOAT2 pos;
	XMFLOAT2 vel;
	float lifeTime;
	float time;
};

class Explosion : public GameEntity
{
public:
	Explosion(const XMVECTOR& position, int size, float startSpeed = 2.f);

	void Update(System* system);
	void Render(Graphics* graphics) const;
private:
	std::list<Particle> particles_;
	std::clock_t frameStartTime_;
	float activeTime_;
	float lastSpawnTime_;
};

#endif // EXPLOSION_H_INCLUDED
