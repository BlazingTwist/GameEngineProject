#include "mainstate.h"

namespace gameState {

    static void printInfo() {
        spdlog::info("Starting Main State.");
        spdlog::info("- Press [1] to enter spring demo");
        spdlog::info("- Press [2] to enter free fall demo");
        spdlog::info("- Press [3] to enter orbit demo");
        spdlog::info("- Press [4] to enter a new main state");
        spdlog::info("- Press [5] to exit this state");

        auto &registry = entity::EntityRegistry::getInstance();
        entity::EntityReference *ref1 = registry.createEntity<entity::ExComp1, entity::ExComp2>(entity::ExComp1(), entity::ExComp2());
        entity::EntityReference *ref2 = registry.createEntity<entity::ExComp2, entity::ExComp3>(entity::ExComp2(), entity::ExComp3());
        entity::EntityReference *ref3 = registry.createEntity<entity::ExComp2>(entity::ExComp2());
        
        spdlog::info("ref1: comp1: {} | comp2: {}", registry.getComponentData<entity::ExComp1>(ref1)->data, registry.getComponentData<entity::ExComp2>(ref1)->data);
        spdlog::info("ref2: comp2: {} | comp3: {}", registry.getComponentData<entity::ExComp2>(ref2)->data, registry.getComponentData<entity::ExComp3>(ref2)->data);
        spdlog::info("ref3: comp2: {} | comp3: {}", registry.getComponentData<entity::ExComp2>(ref3)->data, registry.getComponentData<entity::ExComp3>(ref3).has_value());

        registry.eraseEntity(ref1);
        registry.eraseEntity(ref2);
        registry.eraseEntity(ref3);
        delete ref1;
        delete ref2;
        delete ref3;
    }

    void MainGameState::initializeControls() {
        hotkey_springDemo_isDown = input::InputManager::isKeyPressed(input::Key::Num1);
        hotkey_freeFallDemo_isDown = input::InputManager::isKeyPressed(input::Key::Num2);
        hotkey_orbitDemo_isDown = input::InputManager::isKeyPressed(input::Key::Num3);
        hotkey_mainState_isDown = input::InputManager::isKeyPressed(input::Key::Num4);
        hotkey_exit_isDown = input::InputManager::isKeyPressed(input::Key::Num5);
    }

    MainGameState::MainGameState() {
        printInfo();
        initializeControls();
    }

    void MainGameState::update(const long long int &deltaMicroseconds) {
        if (!hotkey_springDemo_isDown && input::InputManager::isKeyPressed(input::Key::Num1)) {
            spdlog::info("main state -> spring demo");
            gameState::GameStateManager::getInstance().startGameState(new gameState::SpringDemoState());
            return;
        }

        if (!hotkey_freeFallDemo_isDown && input::InputManager::isKeyPressed(input::Key::Num2)) {
            spdlog::info("main state -> free fall demo");
            gameState::GameStateManager::getInstance().startGameState(new gameState::FreeFallDemoState());
            return;
        }

        if (!hotkey_orbitDemo_isDown && input::InputManager::isKeyPressed(input::Key::Num3)) {
            spdlog::info("main state -> orbit demo");
            gameState::GameStateManager::getInstance().startGameState(new gameState::OrbitDemoState());
            return;
        }

        if (!hotkey_mainState_isDown && input::InputManager::isKeyPressed(input::Key::Num4)) {
            spdlog::info("main state -> main state");
            gameState::GameStateManager::getInstance().startGameState(new MainGameState());
            return;
        }

        if (!hotkey_exit_isDown && input::InputManager::isKeyPressed(input::Key::Num5)) {
            spdlog::info("main state FINISHED");
            _isFinished = true;
            return;
        }

        initializeControls();
    }

    void MainGameState::draw(const long long int &deltaMicroseconds) {}

    void MainGameState::onResume() {
        printInfo();
        initializeControls();
    }

    void MainGameState::onPause() {
        spdlog::info("===== Main State paused =====");
    }

}