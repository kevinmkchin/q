#pragma once

#include "Common.h"
#include "SDL.h"

class GameRenderer;

class EngineClass
{
public:
	int Run();

private:
	bool SDLInitialize();

	void SDLProcessEvents();

public:

	SDL_Window* window = nullptr;
	SDL_GLContext openglContext = nullptr;
	bool programShouldShutdown = false;

	GameRenderer* gfx = nullptr;
	// World* world;

};

extern EngineClass Engine;
