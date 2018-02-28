#include "Graphics.h"
#include <vector>


Graphics::Graphics(int screenWidth, int screenHeight):
	m_ScreenWidth(screenWidth), m_ScreenHeight(screenHeight)
{
	m_DXManager = NULL;
}


Graphics::~Graphics()
{
	if (m_DXManager)
	{
		delete m_DXManager; 
		m_DXManager = NULL;
	}
}

bool Graphics::InitialiseDx(HWND hwnd)
{
	m_DXManager = new DXManager();
	if (!m_DXManager->Initialise(m_ScreenWidth, m_ScreenHeight, false, hwnd))
		return false;
	else
		return true;
}

void Graphics::Initialise()
{
}

void Graphics::BeginScene(std::vector<unsigned int> screen)
{
	m_DXManager->BeginScene(screen);
}

void Graphics::EndScene()
{
	m_DXManager->EndScene();
}

DXManager * Graphics::GetDXManager()
{
	return m_DXManager;
}

HWND Graphics::GetHwnd()
{
	return m_Hwnd;
}

ID3D11Device * Graphics::GetDevice()
{
	return m_DXManager->GetDevice();
}

ID3D11DeviceContext * Graphics::GetDeviceContext()
{
	return m_DXManager->GetDeviceContext();
}

void Graphics::SetHwnd(HWND hWnd)
{
	m_Hwnd = hWnd;
}
