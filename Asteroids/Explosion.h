#ifndef EXPLOSION_H_INCLUDED
#define EXPLOSION_H_INCLUDED

#include <list>
#include <ctime>
#include "GameEntity.h"

struct Particle
	{
		D3DXVECTOR2 pos;
		D3DXVECTOR2 vel;
		float lifeTime;
		float time;
	};

class Explosion : public GameEntity
{
public:
	Explosion(const D3DXVECTOR3 &position, int size, float startSpeed = 2.f);

	void Update(System* system);
	void Render(Graphics* graphics) const;
private:
	std::list<Particle> particles_;
	std::clock_t frameStartTime_;
	float activeTime_;
	float lastSpawnTime_;
};

#endif // EXPLOSION_H_INCLUDED
