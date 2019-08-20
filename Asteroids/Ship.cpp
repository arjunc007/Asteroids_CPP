#include "Ship.h"
#include "Graphics.h"
#include "Maths.h"
#include "ImmediateMode.h"
#include "ImmediateModeVertex.h"
#include "Random.h"
#include <algorithm>

Ship::Ship() :
	accelerationControl_(0.0f),
	rotationControl_(0.0f),
	velocity_(XMFLOAT3(0.0f, 0.0f, 0.0f)),
	forward_(XMFLOAT3(0.0f, 1.0f, 0.0f)),
	rotation_(0.0f),
	color_(XMVectorSet(1.f, 1.f, 1.f, 1.f)),
	coolDown_(0.f),
	lastShotTime_(std::clock()),
	shotReady_(false)//**TODO: Candidate for crash
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
	if (!shotReady_)
	{
		if ((static_cast<double>(std::clock()) - lastShotTime_) / static_cast<double>(CLOCKS_PER_SEC) > coolDown_)
		{
			shotReady_ = true;
		}
	}

	const float RATE_OF_ROTATION = 0.1f;
	const float MAX_SPEED = 2.0f;
	const float VELOCITY_TWEEN = 0.05f;

	rotation_ = Maths::WrapModulo(rotation_ + rotationControl_ * RATE_OF_ROTATION,
		Maths::TWO_PI);

	XMMATRIX rotationMatrix = XMMatrixRotationZ(rotation_);
	XMVECTOR newForward = XMVector3TransformNormal(XMVectorSet(0.f, 1.0f, 0.0f, 0.0f), rotationMatrix);
	newForward = XMVector3Normalize(newForward);
	XMStoreFloat3(&forward_, newForward);

	XMVECTOR idealVelocity = XMVectorScale(XMLoadFloat3(&forward_), accelerationControl_ * MAX_SPEED);
	XMVECTOR newVelocity = XMVectorLerp(XMLoadFloat3(&velocity_), idealVelocity, VELOCITY_TWEEN);
	XMStoreFloat3(&velocity_, newVelocity);

	XMVECTOR position = GetPosition();
	position = XMVectorAdd(position, XMLoadFloat3(&velocity_));
	SetPosition(position);
}

void Ship::Render(Graphics *graphics) const
{
	ImmediateModeVertex axis[8] =
	{
		{0.0f, -5.0f, 0.0f, 0xffffffff}, {0.0f, 10.0f, 0.0f, 0xffffffff},
		{-5.0f, 0.0f, 0.0f, 0xffffffff}, {5.0f, 0.0f, 0.0f, 0xffffffff},
		{0.0f, 10.0f, 0.0f, 0xffffffff}, {-5.0f, 5.0f, 0.0f, 0xffffffff},
		{0.0f, 10.0f, 0.0f, 0xffffffff}, {5.0f, 5.0f, 0.0f, 0xffffffff},
	};

	XMMATRIX rotationMatrix = XMMatrixRotationZ(rotation_);

	XMVECTOR position = GetPosition();
	XMMATRIX translationMatrix = XMMatrixTranslation(
		XMVectorGetX(position),
		XMVectorGetY(position),
		XMVectorGetZ(position));

	XMMATRIX shipTransform = rotationMatrix * translationMatrix;

	ImmediateMode *immediateGraphics = graphics->GetImmediateMode();

	immediateGraphics->SetModelMatrix(shipTransform);
	immediateGraphics->Draw(D3D11_PRIMITIVE_TOPOLOGY_LINELIST,
		&axis[0],
		8);
	immediateGraphics->SetModelMatrix(XMMatrixIdentity());

	//If velocity not zero, draw exhaust particles
	if (accelerationControl_)
	{
		const int numParticles = 25;
		ImmediateModeVertex exhaust[numParticles];

		//Pick points inside triangle from below ship A(0, -5), B(-10, -30), C(10, -30)
		XMFLOAT2 v1(-10 - 0, -30 - (-5));	//Vector from top vertex to bottom left(B - A)
		XMFLOAT2 v2(10 - 0, -30 - (-5));	//(C - A)

		uint32_t baseColor(0xff3399FF); //Default color for exhaust 

		//Generate points
		for (int i = 0; i < numParticles; i++)
		{
			//Generate points in quad
			exhaust[i].x = Random::GetFloat(1.f) * v1.x + Random::GetFloat(1.f) * v2.x;
			exhaust[i].y = Random::GetFloat(1.f) * v1.y + Random::GetFloat(1.f) * v2.y;

			//If points below y=-15, Bring into exhaust triangle
			if (exhaust[i].y < -30.f)
			{
				exhaust[i].y += 25.f;
			}

			exhaust[i].z = 0.f;

			float factor = (1 - (exhaust[i].y - (-30)) / 25);
			exhaust[i].diffuse = baseColor * (((exhaust[i].y - (-30)) / 25));
		}

		immediateGraphics->SetModelMatrix(shipTransform);
		immediateGraphics->Draw(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST,
			&exhaust[0],
			numParticles);
		immediateGraphics->SetModelMatrix(XMMatrixIdentity());
	}
}

XMVECTOR Ship::GetForwardVector() const
{
	return XMLoadFloat3(&forward_);
}

XMVECTOR Ship::GetVelocity() const
{
	return XMLoadFloat3(&velocity_);
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

void Ship::SetColor(const XMVECTOR& color)
{
	color_ = color;
}

void Ship::Reset()
{
	accelerationControl_ = 0.0f;
	rotationControl_ = 0.0f;

	velocity_ = XMFLOAT3(0.0f, 0.0f, 0.0f);
	forward_ = XMFLOAT3(0.0f, 1.0f, 0.0f);
	rotation_ = 0.0f;

	SetPosition(XMVectorZero());
}
