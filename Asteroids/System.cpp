#include "System.h"
#include "MainWindow.h"
#include "ResourceLoader.h"
#include "Graphics.h"
#include "AssetLoader.h"
#include "StateLibrary.h"
#include "Keyboard.h"
#include "GameState.h"
#include "Game.h"

System::System(HINSTANCE hInstance) :
	moduleInstance_(hInstance),
	mainWindow_(0),
	quit_(false),
	resourceLoader_(0),
	graphics_(0),
	assetLoader_(0),
	stateLibrary_(0),
	keyboard_(0),
	mouse_(nullptr),
	currentState_(0),
	nextState_(0),
	game_(0)
{
}

System::~System()
{
}

void System::Initialise()
{
	mainWindow_ = new MainWindow(moduleInstance_);
	resourceLoader_ = new ResourceLoader();
	graphics_ = Graphics::CreateDevice(mainWindow_->GetHandle(), resourceLoader_);
	assetLoader_ = new AssetLoader();
	stateLibrary_ = new StateLibrary();
	keyboard_ = new Keyboard();
	mouse_ = std::make_unique<DirectX::Mouse>();
	mouse_->SetWindow(mainWindow_->GetHandle());
	game_ = new Game();
}

void System::Test()
{
}

void System::Run()
{
	while (!quit_)
	{
		ProcessMessageQueue();
		SwapState();
		Update();
		Render();
	}

	if (currentState_ != 0)
	{
		currentState_->OnDeactivate(this);
	}
}

void System::Terminate()
{
	delete game_;
	game_ = 0;

	delete keyboard_;
	keyboard_ = 0;

	delete stateLibrary_;
	stateLibrary_ = 0;

	delete assetLoader_;
	assetLoader_ = 0;

	Graphics::DestroyDevice(graphics_);
	graphics_ = 0;

	delete resourceLoader_;
	resourceLoader_ = 0;

	delete mainWindow_;
	mainWindow_ = 0;
}

ResourceLoader *System::GetResourceLoader() const
{
	return resourceLoader_;
}

Graphics *System::GetGraphics() const
{
	return graphics_;
}

AssetLoader *System::GetAssetLoader() const
{
	return assetLoader_;
}

Keyboard *System::GetKeyboard() const
{
	return keyboard_;
}

DirectX::Mouse* System::GetMouse() const
{
	return mouse_.get();
}

Game *System::GetGame() const
{
	return game_;
}

void System::SetNextState(const std::string &stateName)
{
	nextState_ = stateLibrary_->GetState(stateName);
	nextStateArgs_.clear();
}

void System::SetNextState(const std::string &stateName, const GameState::StateArgumentMap &args)
{
	nextState_ = stateLibrary_->GetState(stateName);
	nextStateArgs_ = args;
}

void System::ProcessMessageQueue()
{
	MSG message;
	ZeroMemory(&message, sizeof(message));
	if (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
	{
		switch (message.message)
		{
		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		case WM_KEYDOWN:
			if (message.wParam == VK_ESCAPE)
			{
				Mouse::Get().SetMode(Mouse::MODE_ABSOLUTE);
				quit_ = true;
			}
			break;
		case WM_QUIT:
			Mouse::Get().SetMode(Mouse::MODE_ABSOLUTE);
			quit_ = true;
			break;

		case WM_ACTIVATEAPP:
		case WM_INPUT:
		case WM_MOUSEMOVE:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_MOUSEWHEEL:
		case WM_XBUTTONDOWN:
		case WM_XBUTTONUP:
		case WM_MOUSEHOVER:
			Mouse::ProcessMessage(message.message, message.wParam, message.lParam);
			break;
		default:
			; // Do nothing
		}

		TranslateMessage(&message);
		DispatchMessage(&message);
	}
}

void System::SwapState()
{
	if (nextState_ != 0)
	{
		if (currentState_ != 0)
		{
			currentState_->OnDeactivate(this);
		}
		currentState_ = nextState_;
		nextState_ = 0;
		currentState_->OnActivate(this, nextStateArgs_);
		nextStateArgs_.clear();
	}
}

void System::Update()
{
	assetLoader_->Update();
	keyboard_->Update();
	currentState_->OnUpdate(this);
	Sleep(1);
}

void System::Render()
{
	graphics_->BeginFrame();
	currentState_->OnRender(this);
	graphics_->EndFrame();
}
