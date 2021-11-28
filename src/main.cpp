#include <engine/graphics/renderer/mesh.hpp>
#include <engine/graphics/renderer/meshrenderer.hpp>
#include <engine/graphics/core/device.hpp>
#include <engine/input/inputmanager.hpp>
#include <engine/graphics/core/opengl.hpp>
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <engine/gamestate/basegamestate.h>
#include <engine/gamestate/gamestatemanager.h>
#include "game/states/mainstate.h"

#include <thread>

// CRT's memory leak detection
#ifndef NDEBUG
#if defined(_MSC_VER)
#define _CRTDBG_MAP_ALLOC

#include <crtdbg.h>
#include <glm/gtc/type_ptr.hpp>

#endif
#endif

int main(int argc, char *argv[]) {
#ifndef NDEBUG
#if defined(_MSC_VER)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    //	_CrtSetBreakAlloc(2760);
#endif
#endif

    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

    graphics::Device::initialize(1366, 1366, false);
    GLFWwindow *window = graphics::Device::getWindow();
    input::InputManager::initialize(window);
    input::InputManager::setCursorMode(input::InputManager::CursorMode::NORMAL);

    graphics::glCall(glEnable, GL_DEPTH_TEST);
    graphics::glCall(glClearColor, 0.05f, 0.0f, 0.05f, 1.f);

    gameState::GameStateManager &stateManager = gameState::GameStateManager::getInstance();
    stateManager.startGameState(new gameState::MainGameState());

    while (!glfwWindowShouldClose(window) && !input::InputManager::isKeyPressed(input::Key::ESCAPE)) {
        auto timeUntilNextEvent = stateManager.updateGameStates(window);
        if(timeUntilNextEvent > 0){
            std::this_thread::sleep_for(std::chrono::microseconds(timeUntilNextEvent));
        }
        glfwPollEvents();
    }

    utils::MeshLoader::clear();
    graphics::Device::close();
    return EXIT_SUCCESS;
}