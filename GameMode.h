#pragma once
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <vector>
#include "TetrisBoard.h"
#include "ConsoleRenderer.h"
#include "InputManager.h"

class GameMode
{
public:
    enum class GameModeType
    {
        Single,
        Multiplayer
    };
    GameMode(ConsoleRenderer& renderer, GameModeType mode);
    ~GameMode() = default;

    void Update(InputManager* im);
    void Draw();

private:
	void SetupSingleMode();
	void SetupMultiplayerMode();

    ConsoleRenderer& mRenderer;
    std::vector<std::shared_ptr<TetrisBoard>> mBoards;
    GameModeType mCurrentMode;

};
