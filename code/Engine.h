#pragma once

#include "Common.h"
#include "SDL.h"

class GameRenderer;
class GameMode;
struct GameState;

class EngineClass
{
public:
	int Run();

private:
	bool SDLInitialize();

	void SDLProcessEvents();

	void Load();

public:

	SDL_Window* window = nullptr;
	SDL_GLContext openglContext = nullptr;
	bool programShouldShutdown = false;

	GameRenderer* gfx = nullptr;
	GameMode* game = nullptr;

public:
	GameState* GetGameState();

};

extern EngineClass Engine;
