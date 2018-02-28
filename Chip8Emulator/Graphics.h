#pragma once
#include "DXManager.h"
#include <vector>

class Graphics
{
public:
	Graphics(int screenWidth, int screenHeight);
	~Graphics();

	bool InitialiseDx(HWND hwnd);
	void Initialise();
	void BeginScene(std::vector<unsigned int> screen);
	void EndScene();


	DXManager* GetDXManager();
	HWND GetHwnd();
	ID3D11Device* GetDevice();
	ID3D11DeviceContext* GetDeviceContext();

	void SetHwnd(HWND hWnd);

private:
	DXManager* m_DXManager;
	HWND m_Hwnd;
	int m_ScreenWidth, m_ScreenHeight;
};

