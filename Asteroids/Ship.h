#ifndef SHIP_H_INCLUDED
#define SHIP_H_INCLUDED

#include <ctime>
#include "GameEntity.h"

class Graphics;

class Ship : public GameEntity
{
public:
	Ship();

	void SetControlInput(float acceleration,
		float rotation);

	void Update(System *system);
	void Render(Graphics *graphics) const;

	XMVECTOR GetForwardVector() const;
	XMVECTOR GetVelocity() const;

	bool ReadyToShoot() const;
	void SetCooldown(float cooldown);
	void DisableShooting();

	void SetColor(const XMVECTOR& color);

	void Reset();

protected:
	float coolDown_;
	std::clock_t lastShotTime_;
	bool shotReady_;

private:

	float accelerationControl_;
	float rotationControl_;

	XMFLOAT3 velocity_;
	XMFLOAT3 forward_;
	float rotation_;
	XMVECTOR color_;
};

#endif // SHIP_H_INCLUDED
