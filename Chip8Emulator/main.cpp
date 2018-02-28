
#include <time.h>
#include <windows.h>
#include "Windowsx.h"
#include <sstream> //for wstringstream
#include <iostream>//for wcin/wcout
#include "Chip8.h"
#include "SDL.h"

Chip8 m_Chip;

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 320;
const int CHIP_WIDTH = 64;
const int CHIP_HEIGHT = 32;

void Quit()
{
	SDL_DestroyWindow(window);
	window = nullptr;

	SDL_DestroyRenderer(renderer);
	renderer = nullptr;


	SDL_Quit();
}

void HandleKey(SDL_KeyboardEvent* key, bool keyDown)
{
	SDL_PumpEvents();
	
	char keyPressed = (char)key->keysym.sym;

	switch(keyPressed)
	{
	case '1': 
		m_Chip.SetKeyState(0, keyDown);
		break;
	case '2':
		m_Chip.SetKeyState(1, keyDown);
		break;
	case '3' : 
		m_Chip.SetKeyState(2, keyDown);
		break;
	case '4':
		m_Chip.SetKeyState(12, keyDown);
		break;
	case 'q':
		m_Chip.SetKeyState(3, keyDown);
		break;
	case 'w':
		m_Chip.SetKeyState(4, keyDown);
		break;
	case 'e':
		m_Chip.SetKeyState(5, keyDown);
		break;
	case 'r':
		m_Chip.SetKeyState(13, keyDown);
		break;
	case 'a':
		m_Chip.SetKeyState(6, keyDown);
		break;
	case 's':
		m_Chip.SetKeyState(7, keyDown);
		break;
	case 'd':
		m_Chip.SetKeyState(8, keyDown);
		break;
	case 'f':
		m_Chip.SetKeyState(14, keyDown);
		break;
	case 'z':
		m_Chip.SetKeyState(10, keyDown);
		break;
	case 'x':
		m_Chip.SetKeyState(9, keyDown);
		break;
	case 'c':
		m_Chip.SetKeyState(11, keyDown);
		break;
	case 'v':
		m_Chip.SetKeyState(15, keyDown);
		break;
	}
}

int wmain()
{
	srand(time(NULL)); 
	rand(); rand(); rand();

	window = SDL_CreateWindow("Chip8 Emulator",SDL_WINDOWPOS_CENTERED , SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if (window == nullptr)
		return 0;

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	m_Chip.Initialise();
	m_Chip.LoadGame("todo");
	SDL_Event ev;

	for (;;)
	{
		//Synchronise the emulator with the speed of
		//the original chip8, 60 ticks per second
		auto timeBegin = GetTickCount();
		bool timer = true;
		while (timer)
		{
			auto timeEnd = GetTickCount();
			auto difference = timeEnd - timeBegin;
			//difference is in milliseconds
			if (difference/100.0f >= 0.0005f) //1.0f / 60.0f)
				timer = false;
		}

		if (SDL_PollEvent(&ev) != 0)
		{
			switch (ev.type)
			{
			case SDL_QUIT:
					Quit();
					return 0;
			case SDL_KEYDOWN:
				HandleKey(&ev.key, true);
				break;
			case SDL_KEYUP:
				HandleKey(&ev.key, false);
				break;
			}

		}


		m_Chip.EmulateCycle();
		
		//if m_DrawFlag == true, update the screen
		if (m_Chip.GetDrawflagState())
		{
			m_Chip.SetDrawflagState(false);

			for(int i = 0; i < CHIP_WIDTH * CHIP_HEIGHT; ++i)
			{
				unsigned char pixel = m_Chip.GetScreen(i);
				if(pixel == 0)
				{
					SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
				}
				else
				{
					SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
				}

				int chipRow = i / CHIP_WIDTH;
				int chipCollumn = i % CHIP_WIDTH;

				for(int j = 0; j < 100; ++j)
				{
					int row = (int)(j / 10) + (chipRow * 10);
					int collumn = (int)(j % 10) + (chipCollumn * 10);
					//int index = row * SCREEN_WIDTH + collumn; 

					SDL_RenderDrawPoint(renderer, collumn, row);
				}

			}

			SDL_RenderPresent(renderer);
		}


	}


}

