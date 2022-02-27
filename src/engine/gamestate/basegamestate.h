#ifndef ACAENGINE_GAMESTATE_H
#define ACAENGINE_GAMESTATE_H

namespace gameState {
    class BaseGameState {

    public:
        [[nodiscard]] const bool &isFinished() { return _isFinished; }

        virtual void update(const long long &deltaMicroseconds) = 0;

        virtual void draw(const long long &deltaMicroseconds) = 0;

        virtual void onResume() = 0;

        virtual void onPause() = 0;
        
        virtual ~BaseGameState() = default;

    protected:
        bool _isFinished = false;

    };
}

#endif //ACAENGINE_GAMESTATE_H
