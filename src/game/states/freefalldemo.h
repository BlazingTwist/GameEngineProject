#ifndef ACAENGINE_FREEFALLDEMO_H
#define ACAENGINE_FREEFALLDEMO_H

#include <engine/gamestate/basegamestate.h>
#include <engine/graphics/lightdata.h>
#include <engine/graphics/camera.hpp>
#include <engine/graphics/core/sampler.hpp>
#include <engine/graphics/renderer/mesh.hpp>
#include <engine/graphics/renderer/meshrenderer.hpp>
#include <engine/graphics/resources.hpp>
#include <engine/input/inputmanager.hpp>
#include <engine/gamestate/gamestatemanager.h>
#include <game/camera/defaultcameracontrols.h>
#include "mainstate.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

namespace gameState {
    class FreeFallDemoState : public gameState::BaseGameState {

    public:
        FreeFallDemoState();

        void update(const long long int &deltaMicroseconds) override;

        void draw(const long long int &deltaMicroseconds) override;

        void onResume() override;

        void onPause() override;

    private:
        graphics::Camera camera;
        GLint worldToCameraMatrixID = 0;
        GLint cameraPositionShaderID = 0;
        
        glm::vec3 ambientLightData;
        graphics::LightData lightData;
        graphics::MeshRenderer meshRenderer;

        graphics::Mesh sphereMesh;

        unsigned int mainSphereID = 0;
        unsigned int transitionSphereID = 0;

        graphics::Program program = graphics::Program();
        GLint glsl_ambient_light = 0;

        bool hotkey_reset_isDown = false;
        bool hotkey_mainState_isDown = false;
        bool hotkey_exit_isDown = false;

        glm::vec3 spherePosition = {0.0f, 0.0f, 0.0f};
        float sphereVelocity = 0.0f;

        void initializeHotkeys();

        void initializeShaders();

        void loadShaders();

        void loadGeometry();

        void initializeScene();

        void bindLighting();
        
        void bindCamera();
        
        void updateSpherePosition();

    };
}

#endif //ACAENGINE_FREEFALLDEMO_H
