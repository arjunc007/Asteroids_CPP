#ifndef BULLET_H_INCLUDED
#define BULLET_H_INCLUDED

#include <ctime>
#include "GameEntity.h"

enum Owner
{
	Player,
	Enemy
};

class Bullet : public GameEntity
{
public:
	Bullet(Owner owner, const XMVECTOR& position,
		const XMVECTOR& direction, const float life = 5.f);

	Owner GetOwner() const;

	void Update(System *system);
	void Render(Graphics *graphics) const;

private:

	Owner owner_;
	XMFLOAT3 velocity_;
	float lifeTime_;
	std::clock_t startTime_;
};

#endif // BULLET_H_INCLUDED
