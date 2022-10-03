#pragma once

#include "Common.h"
#include <SDL.h>

class GameRenderer;
class GameRuntime;
struct GameState;

class EngineClass
{
public:
	int Run();

private:
	bool SDLInitialize();

	void SDLProcessEvents();

	void LoadPerpetualResources();

public:
	SDL_Window* window = nullptr;
	SDL_GLContext openglContext = nullptr;
	bool programShouldShutdown = false;

	GameRenderer* gfx = nullptr;
	GameRuntime* game = nullptr;

private:
	int profilerLevel = 0;

private:
	void RegisterConsoleCommandsAndVariables();
	void DrawProfilerGUI();
};

extern EngineClass Engine;
