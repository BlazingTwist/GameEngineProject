#ifndef ACAENGINE_ORBITDEMO_H
#define ACAENGINE_ORBITDEMO_H

#include <engine/gamestate/basegamestate.h>
#include <spdlog/spdlog.h>

namespace gameState {
    class OrbitDemoState : public gameState::BaseGameState {

    public:
        OrbitDemoState();

        void update(const long long int &deltaMicroseconds) override;

        void draw(const long long int &deltaMicroseconds) override;

        void onResume() override;

        void onPause() override;

    };
}

#endif //ACAENGINE_ORBITDEMO_H
