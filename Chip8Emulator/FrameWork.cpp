#include "FrameWork.h"
#include "Chip8.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


Chip8 m_Chip;

FrameWork::FrameWork()
{
	m_Screen.resize(640 * 320);
	for (size_t i = 0; i < m_Screen.size(); i++)
	{
		m_Screen[i] = 0xFFFFFFFF;
	}
}


FrameWork::~FrameWork()
{
	//UnregisterClass(m_ApplicationName, m_hInstance);
	m_hInstance = NULL;
	Engine::GetEngine()->Release();
}

bool FrameWork::Initialise()
{
	m_Chip.Initialise();
	m_Chip.LoadGame("todo");

	if (!CreateDXWindow())
	{
		return false;
	}
	if (!Engine::GetEngine()->Initialise(m_hInstance, Engine::GetEngine()->GetGraphics()->GetHwnd()))
	{
		return false;
	}
	else
		return true;
}

void FrameWork::Update()
{
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));

	//Loop
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			if (m_EmulateNextTick)
			{
				EmulateTick();
				m_EmulateNextTick = false;
				m_TimeBegin = GetTickCount();
			}
			Engine::GetEngine()->Run(m_Screen);
			//Synchronise the emulator with the speed of
			//the original chip8, 60 ticks per second
			auto timeEnd = GetTickCount();
			auto difference = timeEnd - m_TimeBegin;
			//difference is in milliseconds
			if (difference / 100 >= 1.0f / 60.0f)
				m_EmulateNextTick = true;
		}
	}

	
}

void FrameWork::EmulateTick()
{
	m_Chip.EmulateCycle();

	//if m_DrawFlag == true, update the screen
	if (m_Chip.GetDrawflagState())
	{
		DrawGraphics();
		m_Chip.SetDrawflagState(false);
	}
}

void FrameWork::DrawGraphics()
{
	const int WIDTH = 64;
	const int HEIGHT = 32;
	for (size_t collumn = 0; collumn < WIDTH; ++collumn)
	{
		for (size_t row = 0; row < HEIGHT; ++row)
		{
			size_t index = row * WIDTH + collumn * 10;
			unsigned char pixel = m_Chip.GetScreen(index);
			unsigned int fillPixel;
			if (pixel = '0')
			{
				fillPixel = 0x00000000;
			}
			else
			{
				fillPixel = 0xFFFFFFFF;
			}
			const int CWIDTH = 10, CHEIGHT = 10;
			for (size_t cellCollumn = 0; cellCollumn < CWIDTH; cellCollumn++)
			{
				
				for (size_t cellRow = 0; cellRow < CHEIGHT; cellRow++)
				{

					size_t cellIndex = cellRow * CWIDTH * cellCollumn;
					size_t currentIndex = (collumn * 10) + ((row + cellRow) * WIDTH) + cellCollumn;
					m_Screen[currentIndex] = fillPixel;
				}
			}
		}
	}


}



bool FrameWork::CreateDXWindow()
{
	HWND hWnd;
	WNDCLASSEX wc;

	m_hInstance = GetModuleHandle(NULL);

	//Setup the window class with default settings
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = m_hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm = LoadIcon(NULL, IDI_WINLOGO);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = m_ApplicationName;
	wc.cbSize = sizeof(WNDCLASSEX);

	if (!RegisterClassEx(&wc))
	{
		MessageBox(NULL, L"RegisterCLassEx() failed", L"Error", MB_OK);
		PostQuitMessage(0);
		return false;
	}

	int x = 500, y = 500;
	int nStyle = WS_OVERLAPPED | WS_SYSMENU | WS_VISIBLE | WS_CAPTION | WS_MINIMIZEBOX;
	hWnd = CreateWindowEx(WS_EX_APPWINDOW, m_ApplicationName, m_ApplicationName, nStyle,x, y, SCREEN_WIDTH, SCREEN_HEIGHT, NULL, NULL, m_hInstance, NULL);
	if (hWnd == NULL)
	{
		MessageBox(NULL, L"CreateWindowEx() failed", L"Error", MB_OK);
		Engine::GetEngine()->Release();
		PostQuitMessage(0);
		return false;
	}
	if (!Engine::GetEngine()->InitialiseGraphics(hWnd))
	{
		MessageBox(hWnd, L"Could not initialise DirectX 11", L"Error", MB_OK);
		Engine::GetEngine()->Release();
		PostQuitMessage(0);
		UnregisterClass(m_ApplicationName,m_hInstance);
		m_hInstance = nullptr;
		DestroyWindow(hWnd);

		return false;
	}

	Engine::GetEngine()->GetGraphics()->SetHwnd(hWnd);

	ShowWindow(hWnd, SW_SHOW);
	SetForegroundWindow(hWnd);
	SetFocus(hWnd);

	return true;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	switch (message)
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
		break;
	case WM_CLOSE:
		PostQuitMessage(0);
		DestroyWindow(hWnd);
		break;
		
	case WM_KEYDOWN:
	{

		char key = (char)wParam;

		switch (key)
		{
		case '1': m_Chip.SetKeyState(1, true);
			break;
		case '2': m_Chip.SetKeyState(2, true);
			break;
		case '3': m_Chip.SetKeyState(3, true);
			break;
		case '4': m_Chip.SetKeyState(12, true);
			break;
		case 'Q': m_Chip.SetKeyState(4, true);
			break;
		case 'W': m_Chip.SetKeyState(5, true);
			break;
		case 'E': m_Chip.SetKeyState(6, true);
			break;
		case 'R': m_Chip.SetKeyState(13, true);
			break;
		case 'A': m_Chip.SetKeyState(7, true);
			break;
		case 'S': m_Chip.SetKeyState(8, true);
			break;
		case 'D': m_Chip.SetKeyState(9, true);
			break;
		case 'F': m_Chip.SetKeyState(14, true);
			break;
		case 'Z': m_Chip.SetKeyState(10, true);
			break;
		case 'X': m_Chip.SetKeyState(0, true);
			break;
		case 'C': m_Chip.SetKeyState(11, true);
			break;
		case 'V': m_Chip.SetKeyState(15, true);
			break;
		default:
			break;
		}

	}

	case WM_KEYUP:
	{
		char key = (char)wParam;
		switch (key)
		{
		case '1': m_Chip.SetKeyState(1, false);
			break;
		case '2': m_Chip.SetKeyState(2, false);
			break;
		case '3': m_Chip.SetKeyState(3, false);
			break;
		case '4': m_Chip.SetKeyState(12, false);
			break;
		case 'Q': m_Chip.SetKeyState(4, false);
			break;
		case 'W': m_Chip.SetKeyState(5, false);
			break;
		case 'E': m_Chip.SetKeyState(6, false);
			break;
		case 'R': m_Chip.SetKeyState(13, false);
			break;
		case 'A': m_Chip.SetKeyState(7, false);
			break;
		case 'S': m_Chip.SetKeyState(8, false);
			break;
		case 'D': m_Chip.SetKeyState(9, false);
			break;
		case 'F': m_Chip.SetKeyState(14, false);
			break;
		case 'Z': m_Chip.SetKeyState(10, false);
			break;
		case 'X': m_Chip.SetKeyState(0, false);
			break;
		case 'C': m_Chip.SetKeyState(11, false);
			break;
		case 'V': m_Chip.SetKeyState(15, false);
			break;
		default:
			break;

		}
	}
	
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;
	}
	return 0;
}