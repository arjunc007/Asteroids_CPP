#include <vector>

#include "Explosion.h"
#include "Graphics.h"
#include "Random.h"

Explosion::Explosion(const D3DXVECTOR3 &position, int size, float startSpeed) :
	frameStartTime_(std::clock()),
	activeTime_(0.f),
	lastSpawnTime_(0.f)
{
	SetPosition(position);

	//Spawn first wave of particles
	Particle temp;

	int numParticles = 50 + size * 5;

	for(int i = 0; i < numParticles; i++)
	{
		temp.time = 0.f;
		temp.lifeTime = size + Random::GetFloat(2.f); 
		temp.vel = startSpeed * D3DXVECTOR2(Random::GetFloat(-1.f, 1.f), Random::GetFloat(-1.f, 1.f));
		temp.pos.x = 0.f;
		temp.pos.y = 0.f;// + 50 * D3DXVECTOR2(Random::GetFloat(-1.f, 1.f), Random::GetFloat(-1.f, 1.f));
		particles_.push_back(temp);
	}
}

void Explosion::Update(System* system)
{
	float deltaTime = (std::clock() - frameStartTime_) / static_cast<float>(CLOCKS_PER_SEC);

	activeTime_ += deltaTime;

	//Spawn 20 new particles every 0.2s
	if(activeTime_ - lastSpawnTime_ > 0.2f)
	{
		Particle temp;
		for(int i = 0; i < 20; i++)
		{
			temp.time = 0.f;
			temp.lifeTime = Random::GetFloat(2.f); 
			temp.vel = 3.f * D3DXVECTOR2(Random::GetFloat(-1.f, 1.f), Random::GetFloat(-1.f, 1.f));
			temp.pos.x = temp.pos.y = 0.f;// + 50 * D3DXVECTOR2(Random::GetFloat(-1.f, 1.f), Random::GetFloat(-1.f, 1.f));
			particles_.push_back(temp);
		}
		lastSpawnTime_ = activeTime_;
	}


	std::list<Particle>::iterator particleIt = particles_.begin();
	while(particleIt != particles_.end())
	{
		if((*particleIt).time > (*particleIt).lifeTime)
		{
			particleIt = particles_.erase(particleIt);
		}
		else
		{
			(*particleIt).pos += (*particleIt).vel * activeTime_;
			(*particleIt).time += deltaTime;
			++particleIt;

		}
	}

	if(particles_.empty() || activeTime_ > 10.f)
	{
		SetAlive(false);
	}
}

void Explosion::Render(Graphics* graphics) const
{

	if(!IsAlive())
		return;

	struct DummyVert
	{
		float x, y, z;
		D3DCOLOR diffuse;
	};

	std::vector<DummyVert> points;
	DummyVert point;

	D3DXCOLOR baseColor(0xFFF54C0F);

	for(auto particle :particles_)
	{
		point.x = particle.pos.x;
		point.y = particle.pos.y;
		point.z = 0;
		point.diffuse = baseColor;
		points.push_back(point);
	}

	D3DXVECTOR3 position = GetPosition();
	D3DXMATRIX explosionTransform;
	D3DXMatrixTranslation(&explosionTransform,
		position.x,
		position.y,
		position.z);

	D3DXMATRIX identityMatrix;
	D3DXMatrixIdentity(&identityMatrix);

	DWORD dummyFvf = D3DFVF_XYZ | D3DFVF_DIFFUSE;
	graphics->SetPointSize(2.f);
		graphics->SetVertexFormat(dummyFvf);
		graphics->DisableLighting();
		graphics->SetModelMatrix(&explosionTransform);
		graphics->DrawImmediate(D3DPT_POINTLIST,
			points.size(),
			&points[0],
			sizeof(point));
		graphics->SetModelMatrix(&identityMatrix);
		graphics->EnableLighting();
}