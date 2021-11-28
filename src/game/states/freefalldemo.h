#ifndef ACAENGINE_FREEFALLDEMO_H
#define ACAENGINE_FREEFALLDEMO_H

#include <engine/gamestate/basegamestate.h>
#include <spdlog/spdlog.h>

namespace gameState {
    class FreeFallDemoState : public gameState::BaseGameState {

    public:
        FreeFallDemoState();

        void update(const long long int &deltaMicroseconds) override;

        void draw(const long long int &deltaMicroseconds) override;

        void onResume() override;

        void onPause() override;

    };
}

#endif //ACAENGINE_FREEFALLDEMO_H
