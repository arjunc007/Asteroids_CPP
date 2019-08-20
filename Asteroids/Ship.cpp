#include "Ship.h"
#include "Graphics.h"
#include "Maths.h"
#include "Random.h"
#include <algorithm>

Ship::Ship() :
	accelerationControl_(0.0f),
	rotationControl_(0.0f),
	velocity_(D3DXVECTOR3(0.0f, 0.0f, 0.0f)),
	forward_(D3DXVECTOR3(0.0f, 1.0f, 0.0f)),
	rotation_(0.0f),
	color_(0xffffffff),
	coolDown_(0.f),
	lastShotTime_(std::clock()),
	shotReady_(false)//**TODO: Candidate for crash
{
}

Ship::~Ship()
{
}

void Ship::SetControlInput(float acceleration,
	float rotation)
{
	accelerationControl_ = acceleration;
	rotationControl_ = rotation;
}

void Ship::Update(System *system)
{
	if(!shotReady_)
	{
		if((std::clock() - lastShotTime_)/ static_cast<double>(CLOCKS_PER_SEC) > coolDown_)
		{
			shotReady_ = true;
		}
	}

	const float RATE_OF_ROTATION = 0.1f;
	const float MAX_SPEED = 2.0f;
	const float VELOCITY_TWEEN = 0.05f;

	rotation_ = Maths::WrapModulo(rotation_ + rotationControl_ * RATE_OF_ROTATION,
		Maths::TWO_PI);

	D3DXMATRIX rotationMatrix;
	D3DXMatrixRotationZ(&rotationMatrix, rotation_);
	D3DXVec3TransformNormal(&forward_, &D3DXVECTOR3(0.0f, 1.0f, 0.0f),
		&rotationMatrix);
	D3DXVec3Normalize(&forward_, &forward_);

	D3DXVECTOR3 idealVelocity;
	D3DXVec3Scale(&idealVelocity, &forward_, accelerationControl_ * MAX_SPEED);
	D3DXVec3Lerp(&velocity_, &velocity_, &idealVelocity, VELOCITY_TWEEN);

	D3DXVECTOR3 newPosition = GetPosition();
	D3DXVec3Add(&newPosition, &newPosition, &velocity_);
	SetPosition(newPosition);
}

void Ship::Render(Graphics *graphics) const
{
	struct DummyVert
	{
		float x, y, z;
		D3DCOLOR diffuse;
	};

	DummyVert axis[8] =
	{
		{0.0f, -5.0f, 0.0f, color_}, {0.0f, 10.0f, 0.0f, color_},
		{-5.0f, 0.0f, 0.0f, color_}, {5.0f, 0.0f, 0.0f, color_},
		{0.0f, 10.0f, 0.0f, color_}, {-5.0f, 5.0f, 0.0f, color_},
		{0.0f, 10.0f, 0.0f, color_}, {5.0f, 5.0f, 0.0f, color_},
	};

	D3DXMATRIX rotationMatrix;
	D3DXMatrixRotationZ(&rotationMatrix, rotation_);

	D3DXVECTOR3 position = GetPosition();
	D3DXMATRIX translationMatrix;
	D3DXMatrixTranslation(&translationMatrix,
		position.x,
		position.y,
		position.z);

	D3DXMATRIX shipTransform = rotationMatrix * translationMatrix;

	D3DXMATRIX identityMatrix;
	D3DXMatrixIdentity(&identityMatrix);

	DWORD dummyFvf = D3DFVF_XYZ | D3DFVF_DIFFUSE;
	graphics->SetVertexFormat(dummyFvf);
	graphics->DisableLighting();
	graphics->SetModelMatrix(&shipTransform);
	graphics->DrawImmediate(D3DPT_LINELIST,
		4,
		&axis[0],
		sizeof(axis[0]));
	graphics->SetModelMatrix(&identityMatrix);
	graphics->EnableLighting();

	//If velocity not zero, draw exhaust particles
	if(accelerationControl_)
	{
		const int numParticles = 25;
		DummyVert exhaust[numParticles];

		//Pick points inside triangle from below ship A(0, -5), B(-10, -30), C(10, -30)
		D3DXVECTOR2 v1(-10-0, -30 - (-5));	//Vector from top vertex to bottom left(B - A)
		D3DXVECTOR2 v2(10-0, -30 - (-5));	//(C - A)

		D3DXCOLOR baseColor(0xff3399FF); //Default color for exhaust 

		//Generate points
		for(int i = 0; i < numParticles; i++)
		{
			//Generate points in quad
			exhaust[i].x = Random::GetFloat(1.f) * v1.x + Random::GetFloat(1.f) * v2.x;
			exhaust[i].y = Random::GetFloat(1.f) * v1.y + Random::GetFloat(1.f) * v2.y;

			//If points below y=-15, Bring into exhaust triangle
			if(exhaust[i].y < -30.f)
			{
				exhaust[i].y += 25.f;
			}

			exhaust[i].z = 0.f;

			float factor = (1 - (exhaust[i].y - (-30)) / 25);
			exhaust[i].diffuse = baseColor * (((exhaust[i].y - (-30)) / 25));
		}

		graphics->SetPointSize(1.5f);
		graphics->SetVertexFormat(dummyFvf);
		graphics->DisableLighting();
		graphics->SetModelMatrix(&shipTransform);
		graphics->DrawImmediate(D3DPT_POINTLIST,
			numParticles,
			exhaust,
			sizeof(exhaust[0]));
		graphics->SetModelMatrix(&identityMatrix);
		graphics->EnableLighting();
	}
}

D3DXVECTOR3 Ship::GetForwardVector() const
{
	return forward_;
}

D3DXVECTOR3 Ship::GetVelocity() const
{
	return velocity_;
}

bool Ship::ReadyToShoot() const
{
	return shotReady_;
}

void Ship::SetCooldown(float coolDown)
{
	coolDown_ = coolDown;
}

void Ship::DisableShooting()
{
	shotReady_ = false;
	lastShotTime_ = std::clock();
}

void Ship::SetColor(const D3DXCOLOR& color)
{
	color_ = color;
}

void Ship::Reset()
{
	accelerationControl_ = 0.0f;
	rotationControl_ = 0.0f;

	velocity_ = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	forward_ = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	rotation_ = 0.0f;

	SetPosition(D3DXVECTOR3(0.0f, 0.0f, 0.0f));

	lastShotTime_ = std::clock();
	shotReady_ = false;
}
