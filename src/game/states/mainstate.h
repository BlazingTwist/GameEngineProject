#ifndef ACAENGINE_MAINSTATE_H
#define ACAENGINE_MAINSTATE_H

#include <engine/gamestate/basegamestate.h>
#include <spdlog/spdlog.h>
#include <engine/input/inputmanager.hpp>
#include <engine/gamestate/gamestatemanager.h>

#include <game/states/freefalldemo.h>
#include <game/states/springdemo.h>
#include <game/states/orbitdemo.h>

#include <engine/entity/entityreference.h>
#include <engine/entity/entityregistry.h>

namespace gameState {
    class MainGameState : public gameState::BaseGameState {

    public:
        MainGameState();
        
        void update(const long long int &deltaMicroseconds) override;

        void draw(const long long int &deltaMicroseconds) override;

        void onResume() override;
        
        void onPause() override;
        
    private:
        bool hotkey_springDemo_isDown = false;
        bool hotkey_freeFallDemo_isDown = false;
        bool hotkey_orbitDemo_isDown = false;
        bool hotkey_mainState_isDown = false;
        bool hotkey_exit_isDown = false;

        void initializeControls();

    };
}

#endif //ACAENGINE_MAINSTATE_H
