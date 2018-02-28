#pragma once

#include "Graphics.h"
#include <vector>

class Engine
{
public:

	~Engine();

	bool InitialiseGraphics(HWND hwnd);
	bool Initialise(HINSTANCE hinstance, HWND hwnd);
	void Run(std::vector<unsigned int>  screen);
	

	void Release();
	static Engine* GetEngine();
	Graphics* GetGraphics();

private:
	Engine();
	void Update();
	void Render(std::vector<unsigned int>  screen);

private:
	Graphics* m_Graphics;
	static Engine* m_Instance;
};

