#ifndef SHIP_H_INCLUDED
#define SHIP_H_INCLUDED

#include <ctime>
#include "GameEntity.h"

class Graphics;

class Ship : public GameEntity
{
public:
	Ship();
	virtual ~Ship();

	void SetControlInput(float acceleration,
		float rotation);

	virtual void Update(System *system);
	virtual void Render(Graphics *graphics) const;

	D3DXVECTOR3 GetForwardVector() const;
	D3DXVECTOR3 GetVelocity() const;

	bool ReadyToShoot() const;
	void SetCooldown(float cooldown);
	void DisableShooting();

	void SetColor(const D3DXCOLOR& color);

	virtual void Reset();

protected:
	float coolDown_;
	std::clock_t lastShotTime_;
	bool shotReady_;

private:

	float accelerationControl_;
	float rotationControl_;

	D3DXVECTOR3 velocity_;
	D3DXVECTOR3 forward_;
	float rotation_;
	D3DXCOLOR color_;

};

#endif // SHIP_H_INCLUDED
