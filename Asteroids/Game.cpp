#include "Game.h"
#include "System.h"
#include "OrthoCamera.h"
#include "Background.h"
#include "Ship.h"
#include "UFO.h"
#include "Asteroid.h"
#include "Explosion.h"
#include "Keyboard.h"
#include "Random.h"
#include "Maths.h"
#include "Bullet.h"
#include "Graphics.h"
#include "FontEngine.h"
#include "Collision.h"
#include <algorithm>
#include <string>

using ButtonState = Mouse::ButtonStateTracker::ButtonState;

Game::Game() :
	camera_(nullptr),
	background_(nullptr),
	score_(0),
	startFrameTime_(std::clock()),
	player_(nullptr),
	enemy_(nullptr),
	collision_(nullptr)
{
	camera_ = new OrthoCamera();
	camera_->SetPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));
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
	while (scoreIt != scorePopups_.end())
	{
		if ((*scoreIt).life > 2.f)
		{
			scoreIt = scorePopups_.erase(scoreIt);
		}
		else
		{
			(*scoreIt).life += deltaTime;
			(*scoreIt).pos.y -= 0.5f;
			(*scoreIt).color = XMVectorSetW((*scoreIt).color, 1 - (*scoreIt).life / 2.f);
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

	if (enemy_)
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

	FontEngine* fontEngine = graphics->GetFontEngine();
	
	std::string scoreText = "Score: " + std::to_string(score_);

	fontEngine->DrawText(scoreText, 0, 600 - 48, 0xffffff00);

	// XMVectorGetX(XMConvertVectorFloatToUInt(popup.color, 0))
	for (auto popup : scorePopups_)
		fontEngine->DrawText("+" + std::to_string(popup.value), popup.pos.x, popup.pos.y,0xff0000ff, FontEngine::FontType::FONT_TYPE_SMALL);
}

void Game::InitialiseLevel(int numAsteroids)
{
	startFrameTime_ = std::clock();
	scorePopups_.clear();
	DeleteAllAsteroids();
	DeleteAllExplosions();

	SpawnPlayer();
	SpawnAsteroids(numAsteroids);

	if (numAsteroids > 2)
		SpawnEnemy();
	else if (numAsteroids > 1)
		SpawnUFOEnemy(numAsteroids);
}

bool Game::IsLevelComplete() const
{
	return (asteroids_.empty() && explosions_.empty());
}

bool Game::IsGameOver() const
{
	return (player_ == nullptr && explosions_.empty());
}

void Game::DoCollision(GameEntity *a, GameEntity *b)
{
	Ship *player = static_cast<Ship *>(a == player_ ? a : (b == player_ ? b : nullptr));
	Ship* enemy = static_cast<Ship*>(a == enemy_ ? a : (b == enemy_ ? b : nullptr));
	Bullet* bullet = static_cast<Bullet*>(IsBullet(a) ? a : (IsBullet(b) ? b : nullptr));
	Asteroid *asteroid = static_cast<Asteroid *>(IsAsteroid(a) ? a : (IsAsteroid(b) ? b : nullptr));

	if (player && asteroid)
	{
		AsteroidHit(asteroid);
		player->SetAlive(false);
	}

	if (player && enemy)
	{
		enemy->SetAlive(false);
		SpawnExplosionAt(enemy->GetPosition(), 3);
		player->SetAlive(false);
		SpawnExplosionAt(player->GetPosition(), 3);
	}

	if (bullet)
	{
		if (asteroid && bullet->GetOwner() == Player)
		{
			AsteroidHit(asteroid);
			bullet->SetAlive(false);

			Score newScore;

			newScore.color = XMVectorSet(1.0f, 0.0f, 0.0f, 1.0f);

			newScore.life = 0;
			//Convert asteroid position to screen space
			newScore.pos.x = XMVectorGetX(asteroid->GetPosition()) + 400;
			newScore.pos.y = -(XMVectorGetY(asteroid->GetPosition()) - 300);
			switch (asteroid->GetSize())
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
		if (enemy && bullet->GetOwner() == Player)
		{
			enemy->SetAlive(false);
			bullet->SetAlive(false);
			SpawnExplosionAt(enemy->GetPosition(), 3);
		}

		if (player && bullet->GetOwner() == Enemy)
		{
			player->SetAlive(false);
			bullet->SetAlive(false);
			SpawnExplosionAt(player->GetPosition(), 3);
		}
	}
}

void Game::SpawnEnemy()
{
	DeleteEnemy();
	enemy_ = new Ship();
	enemy_->SetPosition(XMVectorSet(-350.f, 250.f, 0.f, 0.f));
	enemy_->SetColor(XMVectorSet(0.f, 0.8f, 0.f, 1.f));
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
	if (enemy_)
	{
		delete enemy_;
		enemy_ = nullptr;
	}
}

void Game::UpdateEnemy(System* system)
{
	if (!enemy_ || !player_)
		return;

	if (enemy_->IsAlive())
	{
		XMVECTOR vectorToPlayer = player_->GetPosition() - enemy_->GetPosition();

		//Check if the enemy is a UFO or a ship enemy
		UFO* ufo = dynamic_cast<UFO*>(enemy_);

		if (ufo)
		{
			if (ufo->ReadyToShoot())
			{
				vectorToPlayer = XMVector3Normalize(vectorToPlayer);
				XMVECTOR bulletPosition = ufo->GetPosition() + vectorToPlayer * 10.0f;
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

			if (XMVectorGetX(XMVector3LengthSq(vectorToPlayer)) > 30000.f)
			{
				//Player not in range, so move closer
				acceleration = 0.7f;
			}
			else if (enemy_->ReadyToShoot())
			{
				//Player in range, shoot
				XMVECTOR enemyForward = enemy_->GetForwardVector();
				XMVECTOR bulletPosition = enemy_->GetPosition() + enemyForward * 10.0f;
				SpawnBullet(Enemy, bulletPosition, enemyForward, 4.f);
				enemy_->DisableShooting();
			}

			vectorToPlayer = XMVector3Normalize(vectorToPlayer);

			//Look Towards player
			float rotation = 0.f;

			XMVECTOR rightVector;
			rightVector = XMVector3Cross(enemy_->GetForwardVector(), XMVectorSet(0.f, 0.f, -1.f, 0.f));

			float dotProduct = XMVectorGetX(XMVector3Dot(rightVector, vectorToPlayer));
			float angleToPlayer = (acos(dotProduct) * 180.f / Maths::PI);

			if (angleToPlayer > 2.f)
			{
				if (dotProduct < 0)
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
	player_ = nullptr;
}

void Game::UpdatePlayer(System *system)
{
	if (!player_)
		return;

	if (player_->IsAlive())
	{
		Keyboard* keyboard = system->GetKeyboard();

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

		Mouse::ButtonStateTracker tracker;
		Mouse::State mouseState = system->GetMouse()->GetState();

		tracker.Update(mouseState);

		if (tracker.leftButton == ButtonState::PRESSED)
		{
			system->GetMouse()->SetMode(Mouse::MODE_RELATIVE);
			system->GetMouse()->SetVisible(true);
		}
		else if (tracker.leftButton == ButtonState::RELEASED)
		{
			system->GetMouse()->SetMode(Mouse::MODE_ABSOLUTE);
		}

		if (mouseState.positionMode == Mouse::MODE_RELATIVE)
		{
			if (mouseState.x > XMVectorGetX(player_->GetPosition()))
				rotation = -1.0f;
			else if(mouseState.x < XMVectorGetX(player_->GetPosition()))
				rotation = 1.0f;
			/*XMVECTOR targetDir = XMVectorSet(float(mouseState.x) - XMVectorGetX(player_->GetPosition()) , float(mouseState.y) - XMVectorGetY(player_->GetPosition()), 0.f, 0.f);

			float crossProductZ = XMVectorGetZ(XMVector3Cross(targetDir, player_->GetForwardVector()));
			if (crossProductZ < 0)
			{
				rotation = -1;
			}
			else if (crossProductZ > 0)
			{
				rotation = 1;
			}*/
		}

		//mouse->SetMode(mouse.leftButton ? Mouse::MODE_RELATIVE : Mouse::MODE_ABSOLUTE);

		player_->SetControlInput(acceleration, rotation);
		player_->Update(system);
		WrapEntity(player_);

		if ((keyboard->IsKeyPressed(VK_SPACE) || tracker.leftButton == ButtonState::PRESSED) && player_->ReadyToShoot())
		{
			XMVECTOR playerForward = player_->GetForwardVector();
			XMVECTOR bulletPosition = player_->GetPosition() + playerForward * 10.0f;
			SpawnBullet(Owner::Player, bulletPosition, playerForward, 4.0f);
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
	while (asteroidIt != asteroids_.end())
	{
		if ((*asteroidIt)->IsAlive())
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
	while (bulletIt != bullets_.end())
	{
		if ((*bulletIt)->IsAlive())
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
	while (explosionIt != explosions_.end())
	{
		if ((*explosionIt)->IsAlive())
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
	XMFLOAT3 entityPosition;
	XMStoreFloat3(&entityPosition, entity->GetPosition());
	entityPosition.x = Maths::WrapModulo(entityPosition.x, -400.0f, 400.0f);
	entityPosition.y = Maths::WrapModulo(entityPosition.y, -300.0f, 300.0f);
	entity->SetPosition(XMLoadFloat3(&entityPosition));
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

void Game::SpawnBullet(Owner owner, const XMVECTOR& position,
	const XMVECTOR& direction, const float life)
{
	Bullet* bullet = new Bullet(owner, position, direction, life);
	bullet->EnableCollisions(collision_, 3.0f);
	bullets_.push_back(bullet);
}

bool Game::IsBullet(GameEntity* entity) const
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
		XMVECTOR position = XMVectorSet(x, y, 0.0f, 0.0f);
		SpawnAsteroidAt(position, 3);
	}
}

void Game::SpawnAsteroidAt(XMVECTOR position, int size)
{
	const float MAX_ASTEROID_SPEED = 1.0f;

	float angle = Random::GetFloat(Maths::TWO_PI);
	XMMATRIX randomRotation = XMMatrixRotationZ(angle);
	XMVECTOR velocity = XMVectorSet(0.0f, Random::GetFloat(MAX_ASTEROID_SPEED), 0.0f, 0.0f);
	velocity = XMVector3TransformNormal(velocity, randomRotation);

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

	SpawnExplosionAt(asteroid->GetPosition(), oldSize);

	if (oldSize > 1)
	{
		int smallerSize = oldSize -1;
		XMVECTOR position = asteroid->GetPosition();
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

void Game::SpawnExplosionAt(const XMVECTOR& position, int size)
{
	const float EXPLOSION_START_SPEED = 5.f;

	Explosion* explosion = new Explosion(position, size, EXPLOSION_START_SPEED);
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
	scorePopups_.clear();
	DeletePlayer();
	DeleteEnemy();
	DeleteAllBullets();
	DeleteAllAsteroids();
	DeleteAllExplosions();
}