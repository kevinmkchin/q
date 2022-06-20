#pragma once

#include "Common.h"
#include "GameComponents.h"
#include "GameState.h"

class GameMode
{
public:
	void LoadAllContent();

	void UnloadAllContent();

	void StartGame();

	void Update();

	GameState state;
};
