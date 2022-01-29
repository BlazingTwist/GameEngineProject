#ifndef ACAENGINE_GAMESTATEMANAGER_H
#define ACAENGINE_GAMESTATEMANAGER_H

#include "basegamestate.h"
#include <vector>
#include <chrono>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <engine/graphics/core/opengl.hpp>

namespace gameState {
    class GameStateManager {

    public:
        static GameStateManager &getInstance() {
            static GameStateManager instance;
            return instance;
        }

        GameStateManager(GameStateManager const &) = delete;

        void operator=(GameStateManager const &) = delete;

        void startGameState(gameState::BaseGameState *baseGameState);

        long long updateGameStates(GLFWwindow *window);

    private:
        GameStateManager() = default;

        std::vector<gameState::BaseGameState *> gameStates = {};
        std::chrono::time_point<std::chrono::high_resolution_clock> lastUpdateTime = std::chrono::high_resolution_clock::now();
        std::chrono::time_point<std::chrono::high_resolution_clock> lastDrawTime = std::chrono::high_resolution_clock::now();

    };
}


#endif //ACAENGINE_GAMESTATEMANAGER_H
