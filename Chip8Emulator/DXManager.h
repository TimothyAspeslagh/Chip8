#pragma once
#include <dxgi.h>
#include <d3dcommon.h>
#include <d3d11.h>
#include <vector>

class DXManager
{
public:
	DXManager();
	~DXManager();
	bool Initialise(int screenWidth, int screenHeight, bool vSync, HWND hwnd);
	void BeginScene(std::vector<unsigned int> screen);
	void EndScene();


	ID3D11Device* GetDevice();
	ID3D11DeviceContext* GetDeviceContext();
private:
	bool InitialiseSwapChain(HWND hwnd, int screenWidth, int screenHeight, unsigned int numerator, unsigned int denominator);
	void InitialiseViewport(int screenWidth, int screenHeight);
	bool InitialiseUAV();
	bool InitialiseStructuredBuffer();
	bool InitialiseTex2D();

private:
	bool m_Vsync_Enabled = false;
	int m_VideoCardMemory;
	char m_VideoCardDescription[128];
	IDXGISwapChain* m_SwapChain;
	ID3D11Device* m_Device;
	ID3D11DeviceContext* m_DeviceContext;
	ID3D11UnorderedAccessView *m_UAV;
	ID3D11Buffer *m_pStructuredBuffer;
	ID3D11Texture2D *m_pTex2D;

	
	
	
	
	
	
	
	
};

