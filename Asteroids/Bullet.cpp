#include "Bullet.h"
#include "Graphics.h"

Bullet::Bullet(Owner owner, const D3DXVECTOR3 &position,
	const D3DXVECTOR3 &direction, const float life) : owner_(owner), 
	startTime_(std::clock()),
	lifeTime_(life)
{
	const float BULLET_SPEED = 4.0f;

	SetPosition(position);
	D3DXVECTOR3 normalisedDirection;
	D3DXVec3Normalize(&normalisedDirection, &direction);
	velocity_ = normalisedDirection * BULLET_SPEED;
}

Owner Bullet::GetOwner() const
{
	return owner_;
}

void Bullet::Update(System *system)
{
	if((std::clock() - startTime_)/ static_cast<double>(CLOCKS_PER_SEC) > lifeTime_)
	{
		SetAlive(false);
		return;
	}

	D3DXVECTOR3 position = GetPosition();
	D3DXVec3Add(&position, &position, &velocity_);
	SetPosition(position);
}

void Bullet::Render(Graphics *graphics) const
{
	const float RADIUS = 3.0f;

	struct DummyVert
	{
		float x, y, z;
		D3DCOLOR diffuse;
	};

	int numLines = 4;
	DummyVert square[5] = {
		{-RADIUS, -RADIUS, 0.0f, 0xffffffff},
		{-RADIUS,  RADIUS, 0.0f, 0xffffffff},
		{ RADIUS,  RADIUS, 0.0f, 0xffffffff},
		{ RADIUS, -RADIUS, 0.0f, 0xffffffff},
		{-RADIUS, -RADIUS, 0.0f, 0xffffffff},
	};
	
	if(owner_ == Enemy)
	{
		numLines = 3;
		square[2].y = 0.f;
		square[3] = square[4];
	}

	D3DXVECTOR3 position = GetPosition();
	D3DXMATRIX translationMatrix;
	D3DXMatrixTranslation(&translationMatrix,
		position.x,
		position.y,
		position.z);

	D3DXMATRIX identityMatrix;
	D3DXMatrixIdentity(&identityMatrix);

	DWORD dummyFvf = D3DFVF_XYZ | D3DFVF_DIFFUSE;
	graphics->SetVertexFormat(dummyFvf);
	graphics->DisableLighting();
	graphics->SetModelMatrix(&translationMatrix);
	graphics->DrawImmediate(D3DPT_LINESTRIP,
		numLines,
		&square[0],
		sizeof(square[0]));
	graphics->SetModelMatrix(&identityMatrix);
	graphics->EnableLighting();
}
