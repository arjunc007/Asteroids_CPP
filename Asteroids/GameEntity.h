#ifndef GAMEENTITY_H_INCLUDED
#define GAMEENTITY_H_INCLUDED

#include <d3dx9math.h>

class System;
class Graphics;
class Collision;
class Collider;

class GameEntity
{
public:
	GameEntity();
	virtual ~GameEntity();

	virtual void Update(System *system);
	virtual void Render(Graphics *graphics) const;

	D3DXVECTOR3 GetPosition() const;
	void SetPosition(const D3DXVECTOR3 &position);

	void EnableCollisions(Collision *collisionSystem, float radius);
	void DisableCollisions();

	bool IsAlive() const;
	void SetAlive(bool b);

private:

	bool HasValidCollider() const;
	void DestroyCollider();

	D3DXVECTOR3 position_;
	Collision *collisionSystem_;
	Collider *collider_;
	bool isAlive_;

};

#endif // GAMEENTITY_H_INCLUDED
