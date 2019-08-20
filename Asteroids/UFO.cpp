#include "UFO.h"
#include "Maths.h"
#include "Graphics.h"
#include "ImmediateMode.h"
#include "ImmediateModeVertex.h"


UFO::UFO(int speed)
{
	speed_ = 20.f * speed;
	DisableShooting();
	startFrameTime_ = std::clock();
	SetCooldown(Maths::WrapModulo(8 - speed, 1.f, 7.f));
	SetPosition(XMVectorSet(-400.0f, 250.0f, 0.0f, 0.0f));
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
		if((static_cast<double>(currentTime) - lastShotTime_)/ static_cast<double>(CLOCKS_PER_SEC) > coolDown_)
		{
			shotReady_ = true;
		}
	}

	XMVECTOR position = GetPosition();
	SetPosition(XMVectorSetX(position, XMVectorGetX(position) + speed_ * deltaTime));

	startFrameTime_ = currentTime;
}

void UFO::Render(Graphics* graphics) const
{
	ImmediateModeVertex axis[14] =
	{
		{-5.0f, 0.0f, 0.0f, 0xffffffff}, {-5.0f, 5.0f, 0.0f, 0xffffffff},
		{-5.0f, 5.0f, 0.0f, 0xffffffff}, {5.0f, 5.0f, 0.0f, 0xffffffff},
		{5.0f, 5.0f, 0.0f, 0xffffffff}, {5.0f, 0.0f, 0.0f, 0xffffffff},
		{5.0f, 0.0f, 0.0f, 0xffffffff}, {-5.0f, 0.0f, 0.0f, 0xffffffff},
		{-5.0f, 0.0f, 0.0f, 0xffffffff}, {-10.0f, -5.0f, 0.0f, 0xffffffff},
		{-10.0f, -5.0f, 0.0f, 0xffffffff}, {10.0f, -5.0f, 0.0f, 0xffffffff},
		{10.0f, -5.0f, 0.0f, 0xffffffff}, {5.0f, 0.0f, 0.0f, 0xffffffff},
	};

	XMVECTOR position = GetPosition();
	XMMATRIX translationMatrix = XMMatrixTranslation(
		XMVectorGetX(position),
		XMVectorGetY(position),
		XMVectorGetZ(position));

	XMMATRIX shipTransform = translationMatrix;

	ImmediateMode* immediateGraphics = graphics->GetImmediateMode();

	immediateGraphics->SetModelMatrix(shipTransform);
	immediateGraphics->Draw(D3D11_PRIMITIVE_TOPOLOGY_LINELIST,
		&axis[0],
		14);
	immediateGraphics->SetModelMatrix(XMMatrixIdentity());
}

void UFO::Reset()
{
	lastShotTime_ = 0.f;
	SetPosition(XMVectorSet(-400.0f, 250.0f, 0.0f, 0.0f));
}