#include "Bullet.h"
#include "Graphics.h"
#include "ImmediateMode.h"
#include "ImmediateModeVertex.h"

Bullet::Bullet(Owner owner, const XMVECTOR& position,
	const XMVECTOR& direction, const float life) : owner_(owner),
	startTime_(std::clock()),
	lifeTime_(life)
{
	const float BULLET_SPEED = 4.0f;

	SetPosition(position);
	XMStoreFloat3(&velocity_, XMVector3Normalize(direction) * BULLET_SPEED);
}

Owner Bullet::GetOwner() const
{
	return owner_;
}

void Bullet::Update(System *system)
{
	if ((static_cast<double>(std::clock()) - startTime_) / static_cast<double>(CLOCKS_PER_SEC) > lifeTime_)
	{
		SetAlive(false);
		return;
	}

	XMVECTOR position = GetPosition();
	position = XMVectorAdd(position, XMLoadFloat3(&velocity_));
	SetPosition(position);
}

void Bullet::Render(Graphics *graphics) const
{
	const float RADIUS = 3.0f;

	int numLines = 4;
	ImmediateModeVertex square[5] =
	{
		{-RADIUS, -RADIUS, 0.0f, 0xffffffff},
		{-RADIUS,  RADIUS, 0.0f, 0xffffffff},
		{ RADIUS,  RADIUS, 0.0f, 0xffffffff},
		{ RADIUS, -RADIUS, 0.0f, 0xffffffff},
		{-RADIUS, -RADIUS, 0.0f, 0xffffffff},
	};

	if (owner_ == Enemy)
	{
		numLines = 3;
		square[2].y = 0.f;
		square[3] = square[4];
	}

	XMVECTOR position = GetPosition();
	XMMATRIX translationMatrix = XMMatrixTranslation(
		XMVectorGetX(position),
		XMVectorGetY(position),
		XMVectorGetZ(position));

	ImmediateMode *immediateGraphics = graphics->GetImmediateMode();

	immediateGraphics->SetModelMatrix(translationMatrix);
	immediateGraphics->Draw(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP,
		&square[0],
		numLines == 4 ? 5 : 4);
	immediateGraphics->SetModelMatrix(XMMatrixIdentity());
}
