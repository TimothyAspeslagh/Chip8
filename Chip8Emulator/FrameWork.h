#pragma once

#include <Windows.h>
#include <iostream>
#include <string>
#include "Engine.h"
#include <vector>

class FrameWork
{
public:
	FrameWork();
	~FrameWork();

	bool Initialise();
	void Update();
	void DrawGraphics();

private:
	bool CreateDXWindow();
	void EmulateTick();
private:
	bool m_EmulateNextTick = true;
	DWORD m_TimeBegin;
	wchar_t* m_ApplicationName = L"Chip8 Emulator";
	HINSTANCE m_hInstance;
	const int SCREEN_WIDTH = 640;
	const int SCREEN_HEIGHT= 320;

	std::vector<unsigned int> m_Screen;

	ID3D11ShaderResourceView* m_pTextureResourceView;
	
};

