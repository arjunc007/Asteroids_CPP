#include <vector>

#include "Explosion.h"
#include "Graphics.h"
#include "Random.h"
#include "ImmediateMode.h"
#include "ImmediateModeVertex.h"

Explosion::Explosion(const XMVECTOR& position, int size, float startSpeed) :
	frameStartTime_(std::clock()),
	activeTime_(0.f),
	lastSpawnTime_(0.f)
{
	SetPosition(position);

	//Spawn first wave of particles
	Particle temp;

	int numParticles = 50 + size * 5;

	for (int i = 0; i < numParticles; i++)
	{
		temp.time = 0.f;
		temp.lifeTime = size + Random::GetFloat(2.f);
		XMStoreFloat2(&temp.vel, startSpeed * XMVectorSet(Random::GetFloat(-1.f, 1.f), Random::GetFloat(-1.f, 1.f), 0.f, 0.f));
		temp.pos.x = 0.f;
		temp.pos.y = 0.f;
		particles_.push_back(temp);
	}
}

void Explosion::Update(System* system)
{
	float deltaTime = (std::clock() - frameStartTime_) / static_cast<float>(CLOCKS_PER_SEC);

	activeTime_ += deltaTime;

	//Spawn 20 new particles every 0.2s
	if (activeTime_ - lastSpawnTime_ > 0.2f)
	{
		Particle temp;
		for (int i = 0; i < 20; i++)
		{
			temp.time = 0.f;
			temp.lifeTime = Random::GetFloat(2.f);
			XMStoreFloat2(&temp.vel, 3.f * XMVectorSet(Random::GetFloat(-1.f, 1.f), Random::GetFloat(-1.f, 1.f), 0.f, 0.f));
			temp.pos.x = temp.pos.y = 0.f;// + 50 * D3DXVECTOR2(Random::GetFloat(-1.f, 1.f), Random::GetFloat(-1.f, 1.f));
			particles_.push_back(temp);
		}
		lastSpawnTime_ = activeTime_;
	}


	std::list<Particle>::iterator particleIt = particles_.begin();
	while (particleIt != particles_.end())
	{
		if ((*particleIt).time > (*particleIt).lifeTime)
		{
			particleIt = particles_.erase(particleIt);
		}
		else
		{
			XMVECTOR newPos = XMLoadFloat2(&(*particleIt).pos);
			newPos += XMLoadFloat2(&(*particleIt).vel) * activeTime_;
			XMStoreFloat2(&(*particleIt).pos, newPos);
			(*particleIt).time += deltaTime;
			++particleIt;

		}
	}

	if (particles_.empty() || activeTime_ > 10.f)
	{
		SetAlive(false);
	}
}

void Explosion::Render(Graphics* graphics) const
{

	if (!IsAlive())
		return;

	std::vector<ImmediateModeVertex> points;
	ImmediateModeVertex point;

	uint32_t baseColor(0xFFF54C0F);

	for (auto particle : particles_)
	{
		point.x = particle.pos.x;
		point.y = particle.pos.y;
		point.z = 0;
		point.diffuse = baseColor;
		points.push_back(point);
	}

	XMVECTOR position = GetPosition();
	XMMATRIX translationMatrix = XMMatrixTranslation(
		XMVectorGetX(position),
		XMVectorGetY(position),
		XMVectorGetZ(position));

	XMMATRIX shipTransform = translationMatrix;

	ImmediateMode* immediateGraphics = graphics->GetImmediateMode();

	immediateGraphics->SetModelMatrix(shipTransform);
	immediateGraphics->Draw(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST,
		&points[0],
		points.size());
	immediateGraphics->SetModelMatrix(XMMatrixIdentity());
}