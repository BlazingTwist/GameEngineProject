#include "gamestatemanager.h"

namespace gameState {

    using clock = std::chrono::high_resolution_clock;
    using microseconds = std::chrono::microseconds;

    static constexpr auto noStateWaitDelay = microseconds(1'000'000 / 10).count();
    static constexpr auto targetUpdateInterval = microseconds(1'000'000 / 30).count();
    static constexpr auto targetDrawInterval = microseconds(1'000'000 / 60).count();

    void GameStateManager::startGameState(gameState::BaseGameState *baseGameState) {
        if (!gameStates.empty()) {
            gameStates.back()->onPause();
        }
        gameStates.push_back(baseGameState);
    }

    long long GameStateManager::updateGameStates(GLFWwindow *window) {
        if (gameStates.empty()) {
            return noStateWaitDelay;
        }

        gameState::BaseGameState *currentGameState = gameStates.back();
        if (currentGameState->isFinished()) {
            gameStates.pop_back();
            delete currentGameState;
            if (gameStates.empty()) {
                return noStateWaitDelay;
            }
            gameStates.back()->onResume();
        }

        auto now = clock::now();
        // TODO if the pc is too slow for the target intervals, we're just building up time deficits that never get depleted

        auto timeUntilUpdate = targetUpdateInterval - std::chrono::duration_cast<microseconds>(now - lastUpdateTime).count();
        if (timeUntilUpdate <= 0) {
            timeUntilUpdate += targetUpdateInterval;
            lastUpdateTime = lastUpdateTime + microseconds(targetUpdateInterval);
            gameStates.back()->update(targetUpdateInterval);
        }

        auto timeUntilDraw = targetDrawInterval - std::chrono::duration_cast<microseconds>(now - lastDrawTime).count();
        if (timeUntilDraw <= 0) {
            graphics::glCall(glClear, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            timeUntilDraw += targetDrawInterval;
            lastDrawTime = lastDrawTime + microseconds(targetDrawInterval);
            gameStates.back()->draw(targetDrawInterval);
            glfwSwapBuffers(window);
            graphics::glCall(glClear, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }

        return std::min(timeUntilUpdate, timeUntilDraw);
    }
}