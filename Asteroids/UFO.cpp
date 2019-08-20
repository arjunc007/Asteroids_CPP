#include "UFO.h"
#include "Maths.h"
#include "Graphics.h"


UFO::UFO(int speed)
{
	speed_ = 20.f * speed;
	DisableShooting();
	startFrameTime_ = std::clock();
	SetCooldown(Maths::WrapModulo(8 - speed, 1.f, 7.f));
	SetPosition(D3DXVECTOR3(-400.0f, 250.0f, 0.0f));
}


UFO::~UFO(void)
{
}

void UFO::Update(System* system)
{
	std::clock_t currentTime = std::clock();
	float deltaTime = (currentTime - startFrameTime_) / static_cast<float>(CLOCKS_PER_SEC);

	if(!shotReady_)
	{
		if((currentTime - lastShotTime_)/ static_cast<double>(CLOCKS_PER_SEC) > coolDown_)
		{
			shotReady_ = true;
		}
	}

	D3DXVECTOR3 position = GetPosition();
	position.x += speed_ * deltaTime;
	SetPosition(position);

	startFrameTime_ = currentTime;
}

void UFO::Render(Graphics* graphics) const
{
	struct DummyVert
	{
		float x, y, z;
		D3DCOLOR diffuse;
	};

	DummyVert axis[14] =
	{
		{-5.0f, 0.0f, 0.0f, 0xffffffff}, {-5.0f, 5.0f, 0.0f, 0xffffffff},
		{-5.0f, 5.0f, 0.0f, 0xffffffff}, {5.0f, 5.0f, 0.0f, 0xffffffff},
		{5.0f, 5.0f, 0.0f, 0xffffffff}, {5.0f, 0.0f, 0.0f, 0xffffffff},
		{5.0f, 0.0f, 0.0f, 0xffffffff}, {-5.0f, 0.0f, 0.0f, 0xffffffff},
		{-5.0f, 0.0f, 0.0f, 0xffffffff}, {-10.0f, -5.0f, 0.0f, 0xffffffff},
		{-10.0f, -5.0f, 0.0f, 0xffffffff}, {10.0f, -5.0f, 0.0f, 0xffffffff},
		{10.0f, -5.0f, 0.0f, 0xffffffff}, {5.0f, 0.0f, 0.0f, 0xffffffff},
	};

	D3DXVECTOR3 position = GetPosition();
	

	D3DXMATRIX shipTransform;
	D3DXMatrixTranslation(&shipTransform,
		position.x,
		position.y,
		position.z);

	D3DXMATRIX identityMatrix;
	D3DXMatrixIdentity(&identityMatrix);

	DWORD dummyFvf = D3DFVF_XYZ | D3DFVF_DIFFUSE;
	graphics->SetVertexFormat(dummyFvf);
	graphics->DisableLighting();
	graphics->SetModelMatrix(&shipTransform);
	graphics->DrawImmediate(D3DPT_LINELIST,
		7,
		&axis[0],
		sizeof(axis[0]));
	graphics->SetModelMatrix(&identityMatrix);
	graphics->EnableLighting();
}

void UFO::Reset()
{
	lastShotTime_ = 0.f;
	SetPosition(D3DXVECTOR3(-400.0f, 250.0f, 0.0f));
}