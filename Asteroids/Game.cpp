#include "Game.h"
#include "System.h"
#include "Graphics.h"
#include "OrthoCamera.h"
#include "Background.h"
#include "Ship.h"
#include "UFO.h"
#include "Asteroid.h"
#include "Explosion.h"
#include "Keyboard.h"
#include "Random.h"
#include "Maths.h"
#include "Collision.h"
#include <algorithm>

Game::Game() :
	camera_(0),
	background_(0),
	score_(0),
	startFrameTime_(std::clock()),
	player_(0),
	enemy_(0),
	collision_(0)
{
	camera_ = new OrthoCamera();
	camera_->SetPosition(D3DXVECTOR3(0.0f, 0.0f, 0.0f));
	camera_->SetFrustum(800.0f, 600.0f, -100.0f, 100.0f);
	background_ = new Background(800.0f, 600.0f);
	collision_ = new Collision();
}

Game::~Game()
{
	delete camera_;
	delete background_;
	delete player_;
	delete enemy_;
	DeleteAllBullets();
	DeleteAllAsteroids();
	DeleteAllExplosions();
	delete collision_;
}

void Game::Update(System *system)
{
	UpdateEnemy(system);

	UpdatePlayer(system);

	UpdateAsteroids(system);

	UpdateExplosions(system);

	UpdateBullets(system);
	
	UpdateCollisions();

	std::clock_t endTime = std::clock();
	float deltaTime = (endTime - startFrameTime_) / static_cast<float>(CLOCKS_PER_SEC);

	//Update score popup list
	std::list<Score>::iterator scoreIt = scorePopups_.begin();
	while(scoreIt != scorePopups_.end())
	{
		if((*scoreIt).life > 2.f)
		{
			scoreIt = scorePopups_.erase(scoreIt);
		}
		else
		{
			(*scoreIt).life += deltaTime;
			(*scoreIt).pos.y -= 0.5f;
			(*scoreIt).color.a = 1 - (*scoreIt).life/2.f;
			++scoreIt;
		}
	}

	startFrameTime_ = endTime;
}

void Game::RenderBackgroundOnly(Graphics *graphics)
{
	camera_->SetAsView(graphics);
	background_->Render(graphics);
}

void Game::RenderEverything(Graphics *graphics)
{
	camera_->SetAsView(graphics);

	background_->Render(graphics);

	if (player_)
	{
		player_->Render(graphics);
	}

	if(enemy_)
	{
		enemy_->Render(graphics);
	}

	for (AsteroidList::const_iterator asteroidIt = asteroids_.begin(),
		end = asteroids_.end();
		asteroidIt != end;
		++asteroidIt)
	{
		(*asteroidIt)->Render(graphics);
	}

	for (BulletList::const_iterator bulletIt = bullets_.begin(), 
		end = bullets_.end(); 
		bulletIt != end; 
		++bulletIt) //**TODO: Candidate for crash
	{
		(*bulletIt)->Render(graphics);
	}

	for (ExplosionList::const_iterator explosionIt = explosions_.begin(),
		end = explosions_.end();
		explosionIt != end;
		++explosionIt)
	{
		(*explosionIt)->Render(graphics);
	}
}

void Game::InitialiseLevel(int numAsteroids)
{
	startFrameTime_ = std::clock();
	//NumAsteroids is the level number
	DeleteAllAsteroids();
	DeleteAllExplosions();

	SpawnPlayer();
	SpawnAsteroids(numAsteroids);

	if(numAsteroids > 4)
		SpawnEnemy();
	else if(numAsteroids > 2)
		SpawnUFOEnemy(numAsteroids);
}

bool Game::IsLevelComplete() const
{
	return (asteroids_.empty() && explosions_.empty());
}

bool Game::IsGameOver() const
{
	return (player_ == 0 && explosions_.empty());
}

void Game::DoCollision(GameEntity *a, GameEntity *b)
{
	Ship *player = static_cast<Ship *>(a == player_ ? a : (b == player_ ? b : 0));
	Ship*enemy = static_cast<Ship *>(a == enemy_ ? a : (b == enemy_ ? b : 0));
	Bullet *bullet = static_cast<Bullet *>(IsBullet(a) ? a : (IsBullet(b) ? b : 0));
	Asteroid *asteroid = static_cast<Asteroid *>(IsAsteroid(a) ? a : (IsAsteroid(b) ? b : 0));

	if (player && asteroid)
	{
		AsteroidHit(asteroid);
		player->SetAlive(false);
	}

	if(player && enemy)
	{
		enemy->SetAlive(false);
		SpawnExplosionAt(enemy->GetPosition(), 3);
		player->SetAlive(false);
		SpawnExplosionAt(player->GetPosition(), 3);
	}

	if (bullet)
	{
		if(asteroid && bullet->GetOwner() == Player)
		{
			AsteroidHit(asteroid);
			bullet->SetAlive(false);

			Score newScore;

			newScore.color = 0xFFFF0000;
			newScore.life = 0;
			//Convert asteroid position to screen space
			newScore.pos.x = asteroid->GetPosition().x + 400;
			newScore.pos.y = -(asteroid->GetPosition().y - 300);
			switch(asteroid->GetSize())
			{
			case 1: score_ += 50;
				newScore.value = 50;
				break;
			case 2: score_ += 20;
				newScore.value = 20;
				break;
			case 3: score_ += 10;
				newScore.value = 10;
				break;
			default:
				break;
			}
			scorePopups_.push_back(newScore);
		}

		if(enemy && bullet->GetOwner() == Player)
		{
			enemy->SetAlive(false);
			bullet->SetAlive(false);
			SpawnExplosionAt(enemy->GetPosition(), 3);
		}

		if(player && bullet->GetOwner() == Enemy)
		{
			player->SetAlive(false);
			bullet->SetAlive(false);
			SpawnExplosionAt(player->GetPosition(), 3);
		}
	}
}

void Game::SpawnPlayer()
{
	DeletePlayer();
	player_ = new Ship();
	player_->SetCooldown(1.f);
	player_->EnableCollisions(collision_, 10.0f);
}

void Game::DeletePlayer()
{
	delete player_;
	player_ = 0;
}

void Game::SpawnEnemy()
{
	DeleteEnemy();
	enemy_ = new Ship();
	enemy_->SetPosition(D3DXVECTOR3(-350.f, 250.f, 0.f));
	enemy_->SetColor(0xff00cc00);
	enemy_->SetCooldown(2.f);
	enemy_->EnableCollisions(collision_, 10.0f);
}

void Game::SpawnUFOEnemy(int level)
{
	DeleteEnemy();
	enemy_ = new UFO(level);
	enemy_->EnableCollisions(collision_, 10.0f);
}

void Game::DeleteEnemy()
{
	if(enemy_)
	{
		delete enemy_;
		enemy_ = 0;
	}
}

void Game::UpdateEnemy(System* system)
{
	if(!enemy_ || !player_)
		return;

	if(enemy_->IsAlive())
	{
		D3DXVECTOR3 vectorToPlayer = player_->GetPosition() - enemy_->GetPosition();

		//Check if the enemy is a UFO or a ship enemy
		UFO* ufo = dynamic_cast<UFO*>(enemy_);

		if(ufo)
		{
			if (ufo->ReadyToShoot())
			{
				D3DXVec3Normalize(&vectorToPlayer, &vectorToPlayer);
				D3DXVECTOR3 bulletPosition = ufo->GetPosition() + vectorToPlayer * 10.0f;
				SpawnBullet(Enemy, bulletPosition, vectorToPlayer, 3.f);
				ufo->DisableShooting();
			}

			ufo->Update(system);
			WrapEntity(ufo);
		}
		else
		{
			//Move Towards player
			float acceleration = 0.f;

			if(D3DXVec3LengthSq(&vectorToPlayer) > 30000.f)
			{
				//Player not in range, so move closer
				acceleration = 0.7f;
			}
			else if (enemy_->ReadyToShoot())
			{
				//Player in range, shoot
				D3DXVECTOR3 enemyForward = enemy_->GetForwardVector();
				D3DXVECTOR3 bulletPosition = enemy_->GetPosition() + enemyForward * 10.0f;
				SpawnBullet(Enemy, bulletPosition, enemyForward, 4.f);
				enemy_->DisableShooting();
			}

			D3DXVec3Normalize(&vectorToPlayer, &vectorToPlayer);

			//Look Towards player
			float rotation = 0.f;

			D3DXVECTOR3 rightVector;
			D3DXVec3Cross(&rightVector, &enemy_->GetForwardVector(), &D3DXVECTOR3(0.f, 0.f, -1.f));

			float dotProduct = D3DXVec3Dot(&rightVector, &vectorToPlayer);
			float angleToPlayer = (acos(dotProduct) * 180.f / Maths::PI);
			
			if(angleToPlayer > 2.f)
			{
				if( dotProduct < 0)
					rotation = -1.f;
				else
					rotation = 1.f;
			}

			enemy_->SetControlInput(acceleration, rotation);
			enemy_->Update(system);
			WrapEntity(enemy_);
		}
	}
	else
		DeleteEnemy();

}

void Game::UpdatePlayer(System *system)
{
	if (player_ == 0)
		return;

	if(player_->IsAlive())
	{

		Keyboard *keyboard = system->GetKeyboard();

		float acceleration = 0.0f;
		if (keyboard->IsKeyHeld(VK_UP) || keyboard->IsKeyHeld('W'))
		{
			acceleration = 1.0f;
		}
		else if (keyboard->IsKeyHeld(VK_DOWN) || keyboard->IsKeyHeld('S'))
		{
			acceleration = -1.0f;
		}

		float rotation = 0.0f;
		if (keyboard->IsKeyHeld(VK_RIGHT) || keyboard->IsKeyHeld('D'))
		{
			rotation = -1.0f;
		}
		else if (keyboard->IsKeyHeld(VK_LEFT) || keyboard->IsKeyHeld('A'))
		{
			rotation = 1.0f;
		}

		player_->SetControlInput(acceleration, rotation);
		player_->Update(system);
		WrapEntity(player_);

		if (keyboard->IsKeyPressed(VK_SPACE) && player_->ReadyToShoot())
		{
			D3DXVECTOR3 playerForward = player_->GetForwardVector();
			D3DXVECTOR3 bulletPosition = player_->GetPosition() + playerForward * 10.0f;
			SpawnBullet(Player, bulletPosition, playerForward, 4.f);
			player_->DisableShooting();
		}
	}
	else
	{
		DeletePlayer();
	}
}

void Game::UpdateAsteroids(System *system)
{
	AsteroidList::const_iterator asteroidIt = asteroids_.begin();
	while(asteroidIt != asteroids_.end())
	{
		if((*asteroidIt)->IsAlive())
		{
			(*asteroidIt)->Update(system);
			WrapEntity(*asteroidIt);
			++asteroidIt;
		}
		else
		{
			DeleteAsteroid(*(asteroidIt++));
		}
	}
}

void Game::UpdateBullets(System *system)
{
	BulletList::const_iterator bulletIt = bullets_.begin();
	while(bulletIt != bullets_.end())
	{
		if((*bulletIt)->IsAlive())
		{
			(*bulletIt)->Update(system);
			WrapEntity((*bulletIt));
			++bulletIt;
		}
		else
		{
			DeleteBullet((*bulletIt++));
		}
	}
}

void Game::UpdateExplosions(System* system)
{
	ExplosionList::const_iterator explosionIt = explosions_.begin();
	while(explosionIt != explosions_.end())
	{
		if((*explosionIt)->IsAlive())
		{
			(*explosionIt)->Update(system);
			WrapEntity(*explosionIt);
			++explosionIt;
		}
		else
		{
			DeleteExplosion((*explosionIt++));
		}
	}
}

void Game::WrapEntity(GameEntity *entity) const
{
	D3DXVECTOR3 entityPosition = entity->GetPosition();
	entityPosition.x = Maths::WrapModulo(entityPosition.x, -400.0f, 400.0f);
	entityPosition.y = Maths::WrapModulo(entityPosition.y, -300.0f, 300.0f);
	entity->SetPosition(entityPosition);
}

void Game::DeleteAllBullets()
{
	for (BulletList::const_iterator bulletIt = bullets_.begin(),
		end = bullets_.end();
		bulletIt != end;
		++bulletIt)
	{
		delete (*bulletIt);
	}

	bullets_.clear();
}

void Game::DeleteAllAsteroids()
{
	for (AsteroidList::const_iterator asteroidIt = asteroids_.begin(),
		end = asteroids_.end();
		asteroidIt != end;
		++asteroidIt)
	{
		delete (*asteroidIt);
	}

	asteroids_.clear();
}

void Game::DeleteAllExplosions()
{
	for (ExplosionList::const_iterator explosionIt = explosions_.begin(),
		end = explosions_.end();
		explosionIt != end;
	++explosionIt)
	{
		delete (*explosionIt);
	}

	explosions_.clear();
}

void Game::SpawnBullet(Owner owner, const D3DXVECTOR3 &position,
	const D3DXVECTOR3 &direction, const float life)
{
	Bullet* bullet = new Bullet(owner, position, direction, life);
	bullet->EnableCollisions(collision_, 3.0f);
	bullets_.push_back(bullet);
}

bool Game::IsBullet(GameEntity *entity) const
{
	return (std::find(bullets_.begin(),
		bullets_.end(), entity) != bullets_.end()); 
}

void Game::DeleteBullet(Bullet* bullet)
{
	bullets_.remove(bullet);
	delete bullet;
}

void Game::DeleteExplosion(Explosion* explosion)
{
	explosions_.remove(explosion);
	delete explosion;
}

void Game::SpawnAsteroids(int numAsteroids)
{
	float halfWidth = 800.0f * 0.5f;
	float halfHeight = 600.0f * 0.5f;
	for (int i = 0; i < numAsteroids; i++)
	{
		float x = Random::GetFloat(-halfWidth, halfWidth);
		float y = Random::GetFloat(-halfHeight, halfHeight);
		D3DXVECTOR3 position = D3DXVECTOR3(x, y, 0.0f);
		SpawnAsteroidAt(position, 3);
	}
}

void Game::SpawnAsteroidAt(const D3DXVECTOR3 &position, int size)
{
	const float MAX_ASTEROID_SPEED = 1.0f;

	float angle = Random::GetFloat(Maths::TWO_PI);
	D3DXMATRIX randomRotation;
	D3DXMatrixRotationZ(&randomRotation, angle);
	D3DXVECTOR3 velocity = D3DXVECTOR3(0.0f, Random::GetFloat(MAX_ASTEROID_SPEED), 0.0f);
	D3DXVec3TransformNormal(&velocity, &velocity, &randomRotation);

	Asteroid *asteroid = new Asteroid(position, velocity, size);
	asteroid->EnableCollisions(collision_, size * 5.0f);
	asteroids_.push_back(asteroid);
}

bool Game::IsAsteroid(GameEntity *entity) const
{
	return (std::find(asteroids_.begin(),
		asteroids_.end(), entity) != asteroids_.end()); 
}

void Game::AsteroidHit(Asteroid *asteroid)
{
	int oldSize = asteroid->GetSize();
	D3DXVECTOR3 position = asteroid->GetPosition();

	SpawnExplosionAt(position, oldSize);

	if (oldSize > 1)
	{
		int smallerSize = oldSize -1;
		SpawnAsteroidAt(position, smallerSize);
		SpawnAsteroidAt(position, smallerSize);
	}
	asteroid->SetAlive(false);
}

void Game::DeleteAsteroid(Asteroid *asteroid)
{
	asteroids_.remove(asteroid);
	delete asteroid;
}

void Game::SpawnExplosionAt(const D3DXVECTOR3 &position, int size)
{
	const float EXPLOSION_START_SPEED = 5.f;

	Explosion *explosion = new Explosion(position, size, EXPLOSION_START_SPEED);
	explosions_.push_back(explosion);
}

void Game::UpdateCollisions()
{
	collision_->DoCollisions(this);
}

int Game::GetScore() const
{
	return score_;
}

const std::list<Game::Score>& Game::GetPopups() const
{
	return scorePopups_;
}

void Game::ResetGame()
{
	score_ = 0;
	DeletePlayer();
	DeleteEnemy();
	DeleteAllBullets();
	DeleteAllAsteroids();
	DeleteAllExplosions();
}