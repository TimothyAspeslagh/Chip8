#include "DXManager.h"
#include <d3d11.h>
#include "DirectXMath.h"
#include <vector>

#pragma comment(lib, "windowscodecs.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

DXManager::DXManager()
{
	m_SwapChain = 0;
	m_Device = 0;
	m_DeviceContext = 0;

}


DXManager::~DXManager()
{
	

	if (m_DeviceContext)
	{
		m_DeviceContext->Release();
		m_DeviceContext = 0;
	}

	if (m_Device)
	{
		m_Device->Release();
		m_Device = 0;
	}
}

bool DXManager::Initialise(int screenWidth, int screenHeight, bool vSync, HWND hwnd)
{
	
	HRESULT result;
	IDXGIFactory* factory;
	IDXGIAdapter* adapter;
	IDXGIOutput* adapterOutput;
	unsigned int numModes, i, numerator =1, denominator = 0, stringLength;
	DXGI_MODE_DESC* displayModeList;
	DXGI_ADAPTER_DESC adapterDesc;
	int error;
	ID3D11Texture2D* backBufferPtr;

	m_Vsync_Enabled = vSync;

	result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
	if (FAILED(result))
	{
		return false;
	}

	result = factory->EnumAdapters(0, &adapter);
	if (FAILED(result))
	{
		return false;
	}

	result = adapter->EnumOutputs(0, &adapterOutput);
	if (FAILED(result))
	{
		return false;
	}

	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
	if (FAILED(result))
	{
		return false;
	}

	displayModeList = new DXGI_MODE_DESC[numModes];

	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList);
	if (FAILED(result))
	{
		return false;
	}

	for (size_t e = 0; e < numModes; e++)
	{
		if (displayModeList[e].Width == (unsigned int)screenWidth)
		{
			if (displayModeList[e].Height == (unsigned int)screenHeight)
			{
				numerator = displayModeList[e].RefreshRate.Numerator;
				denominator = displayModeList[e].RefreshRate.Denominator;
			}
		}
	}
	if (numerator == 0 && denominator == 0)
	{
		return false;
	}

	result = adapter->GetDesc(&adapterDesc);
	if (FAILED(result))
	{
		return false;
	}

	m_VideoCardMemory = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);
	error = wcstombs_s(&stringLength, m_VideoCardDescription, 128, adapterDesc.Description, 128);
	if (error != 0)
	{
		return false;
	}

	//Release memory
	delete[] displayModeList;
	displayModeList = 0;

	adapterOutput->Release();
	adapterOutput = 0;

	adapter->Release();
	adapter = 0;

	factory->Release();
	factory = 0;

	if (!InitialiseSwapChain(hwnd, screenWidth, screenHeight, numerator, denominator))
	{
		return false;
	}

	/*
	if (!InitialiseUAV())
	{
		return false;
	}
	*/



	result = m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr);
	if (FAILED(result))
	{
		return false;
	}

	backBufferPtr->Release();
	backBufferPtr = 0;

	if (!InitialiseTex2D())
	{
		return false;
	}

	//if (!InitialiseStructuredBuffer())
	//{
	//	return false;
	//}

	InitialiseViewport(screenWidth, screenHeight);

	return true;
}

void DXManager::BeginScene(std::vector<unsigned int> screen)
{
	ID3D11Texture2D *tex;
	
	m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_pTex2D));
	//m_DeviceContext->UpdateSubresource(tex, 0, NULL, reinterpret_cast<void *> (screen->data()), sizeof(unsigned int) * screen->size(), 0);
	
	D3D11_MAPPED_SUBRESOURCE * resource = new D3D11_MAPPED_SUBRESOURCE();
	resource->pData = screen.data();
	resource->DepthPitch = sizeof(UINT);
	resource->RowPitch = sizeof(UINT);
	

	//D3D11_SUBRESOURCE_DATA init = { 0 };
	//init.pSysMem = screen.data()
	m_DeviceContext->Map(m_pTex2D, NULL, D3D11_MAP_WRITE_DISCARD,NULL, resource); // tex vervangen naar de vervanger van structuredbuffer
	
	memcpy_s(resource->pData, sizeof(UINT) * screen.size(), screen.data(), sizeof(UINT) * screen.size());
	m_DeviceContext->Unmap(m_pTex2D, NULL);

	//memcpy_s(m_pTex2D, sizeof(UINT) * screen.size(), screen.data(), sizeof(UINT) *screen.size());
	//memcpy(m_pTex2D, screen.data(), screen.size());
	//m_DeviceContext->CopyResource(m_pTex2D, tex); // copy naar vervang structuredbuffer naar tex
	tex->Release();
	//delete resource;
	//resource = nullptr;
}

bool DXManager::InitialiseUAV()
{
	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	HRESULT result;
	uavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;
	
	result = m_Device->CreateUnorderedAccessView(m_pStructuredBuffer, &uavDesc, &m_UAV);
	if (FAILED(result))
	{
		return false;
	}
	else return true;
}

bool DXManager::InitialiseTex2D()
{
	D3D11_TEXTURE2D_DESC texDesc;
	HRESULT result;
	texDesc.ArraySize = 1;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
	texDesc.CPUAccessFlags = 0;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.Height = 320;
	texDesc.Width = 640;
	texDesc.MipLevels = 0;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0; //Multisampling off
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.MiscFlags = 0;

	result = m_Device->CreateTexture2D(&texDesc, NULL, &m_pTex2D);

	if (FAILED(result))
	{
		return false;
	}
	else
		return true;
}

bool DXManager::InitialiseStructuredBuffer()
{
	D3D11_BUFFER_DESC bufferDesc;
	HRESULT result;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = sizeof(UINT) * (320 * 640);
	bufferDesc.CPUAccessFlags = NULL;
	bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	bufferDesc.StructureByteStride = sizeof(UINT);

	result = m_Device->CreateBuffer(&bufferDesc,NULL , &m_pStructuredBuffer);
	
	if (FAILED(result))
	{
		return false;
	}
	else
		return true;
}

void DXManager::EndScene()
{
	if (m_Vsync_Enabled)
	{
		m_SwapChain->Present(1, 0);
	}
	else
	{
		m_SwapChain->Present(0, 0);
	}

}
ID3D11Device * DXManager::GetDevice()
{
	return m_Device;
}

ID3D11DeviceContext * DXManager::GetDeviceContext()
{
	return m_DeviceContext;
}

bool DXManager::InitialiseSwapChain(HWND hwnd, int screenWidth, int screenHeight, unsigned int numerator, unsigned int denominator)
{
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	D3D_FEATURE_LEVEL featureLevel;
	HRESULT result;

	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

	//Fill the swapChain desc
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Width = screenWidth;
	swapChainDesc.BufferDesc.Height = screenHeight;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	
	if (m_Vsync_Enabled)
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = numerator;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
	}
	else
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	}

	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = hwnd;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0; //Multisampling off
	swapChainDesc.Windowed = true;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = 0;
	featureLevel = D3D_FEATURE_LEVEL_11_0;
	
	auto featurelevel = D3D_FEATURE_LEVEL_11_0;

	result = D3D11CreateDeviceAndSwapChain(
		NULL, 
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		D3D11_CREATE_DEVICE_DEBUG,
		&featureLevel,
		1,
		D3D11_SDK_VERSION,
		&swapChainDesc,
		&m_SwapChain,
		&m_Device,
		&featurelevel,
		&m_DeviceContext);
	if(FAILED(result))
		return false;
	else 
		return true;
}

void DXManager::InitialiseViewport(int screenWidth, int screenHeight)
{
	D3D11_VIEWPORT viewPort;
	
	viewPort.Width = (float)screenWidth;
	viewPort.Height = (float)screenHeight;
	viewPort.MinDepth = 0.0f;
	viewPort.MaxDepth = 1.0f;
	viewPort.TopLeftX = 0.0f;
	viewPort.TopLeftY = 0.0f;

	m_DeviceContext->RSSetViewports(1, &viewPort);
}

