#ifndef GAME_H_INCLUDED
#define GAME_H_INCLUDED

#include <DirectXMath.h>
#include <list>
#include <ctime>

#include "Bullet.h"

using namespace DirectX;

class OrthoCamera;
class Background;
class Ship;
class Bullet;
class Asteroid;
class Explosion;
class Collision;
class System;
class Graphics;
class GameEntity;

class Game
{
public:
	struct Score
	{
		int value;
		XMFLOAT2 pos;
		float life;
		XMVECTOR color;
	};

	Game();
	~Game();

	void Update(System *system);
	void RenderBackgroundOnly(Graphics *graphics);
	void RenderEverything(Graphics *graphics);

	void InitialiseLevel(int numAsteroids);
	bool IsLevelComplete() const;
	bool IsGameOver() const;
	int GetScore() const;
	const std::list<Score>& GetPopups() const;

	void DoCollision(GameEntity *a, GameEntity *b);

	void ResetGame();

private:
	Game(const Game &);
	void operator=(const Game &);

	typedef std::list<Asteroid *> AsteroidList;
	typedef std::list<Explosion *> ExplosionList;
	typedef std::list<Bullet*> BulletList;

	void SpawnPlayer();
	void DeletePlayer();
	void UpdatePlayer(System* system);

	void SpawnEnemy();
	void SpawnUFOEnemy(int level);
	void DeleteEnemy();
	void UpdateEnemy(System* system);

	void UpdateAsteroids(System *system);
	void UpdateBullets(System* system);
	void UpdateExplosions(System* system);
	void WrapEntity(GameEntity *entity) const;

	void DeleteAllBullets();
	void DeleteAllAsteroids();
	void DeleteAllExplosions();

	void SpawnBullet(Owner owner, const XMVECTOR &position,
		const XMVECTOR &direction, const float life);
	bool IsBullet(GameEntity* entity) const; 
	void DeleteBullet(Bullet* bullet);

	void SpawnAsteroids(int numAsteroids);
	void SpawnAsteroidAt(XMVECTOR position, int size);
	bool IsAsteroid(GameEntity *entity) const;
	void AsteroidHit(Asteroid *asteroid);
	void DeleteAsteroid(Asteroid *asteroid);

	void SpawnExplosionAt(const XMVECTOR& position, int size);
	void DeleteExplosion(Explosion* explosion);

	void UpdateCollisions();

	void ShowScore(const int score, const XMVECTOR& position) const;

	OrthoCamera *camera_;

	Background *background_;
	Ship *player_;
	Ship* enemy_;
	BulletList bullets_;
	AsteroidList asteroids_;
	ExplosionList explosions_;

	Collision *collision_;

	int score_;
	std::list<Score> scorePopups_;

	std::clock_t startFrameTime_;
};

#endif // GAME_H_INCLUDED
