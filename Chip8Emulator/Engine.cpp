#include "Engine.h"
#include <vector>

Engine* Engine::m_Instance = nullptr;

Engine::Engine()
{
	m_Graphics = nullptr;
}


Engine::~Engine()
{
	if (m_Graphics)
	{
		delete m_Graphics;
		m_Graphics = nullptr;
	}
}

bool Engine::InitialiseGraphics(HWND hwnd)
{
	m_Graphics = new Graphics(640,320);
	return m_Graphics->InitialiseDx(hwnd);
}

bool Engine::Initialise(HINSTANCE hinstance, HWND hwnd)
{
	 m_Graphics->Initialise();
	 return true;
}

void Engine::Run(std::vector<unsigned int> screen)
{
	Update();
	Render(screen);
}


void Engine::Update()
{
}

void Engine::Render(std::vector<unsigned int> screen)
{
	m_Graphics->BeginScene(screen);

	//Rendering here


	m_Graphics->EndScene();
}


void Engine::Release()
{
	if (m_Instance)
	{
		delete m_Instance;
		m_Instance = nullptr;
	}
}

Engine * Engine::GetEngine()
{
	if (m_Instance == nullptr)
	{
		m_Instance = new Engine();

	}

	return m_Instance;
}

Graphics * Engine::GetGraphics()
{
	return m_Graphics;
}

