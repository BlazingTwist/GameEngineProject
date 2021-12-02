#ifndef ACAENGINE_ORBITDEMO_H
#define ACAENGINE_ORBITDEMO_H

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
    class OrbitDemoState : public gameState::BaseGameState {

    public:
        OrbitDemoState();

        void update(const long long int &deltaMicroseconds) override;

        void draw(const long long int &deltaMicroseconds) override;

        void onResume() override;

        void onPause() override;

    private:
        game::DefaultCameraControls cameraControls;
        
        glm::vec3 ambientLightData;
        graphics::LightData lightData;
        graphics::MeshRenderer meshRenderer;

        graphics::Mesh sphereMesh;
        utils::MeshData *sphereInvertedMeshData;
        graphics::Mesh *sphereInvertedMesh = nullptr;

        unsigned int planetID = 0;
        unsigned int sunID = 0;

        graphics::Program program = graphics::Program();
        GLint glsl_ambient_light = 0;

        bool hotkey_reset_isDown = false;
        bool hotkey_mainState_isDown = false;
        bool hotkey_exit_isDown = false;

        glm::vec3 planetPosition = {0.0f, 0.0f, 0.0f};
        glm::vec3 planetVelocity = {0.0f, 0.0f, 0.0f};
        glm::vec3 sunPosition = {0.0f, 0.0f, 0.0f};
        glm::vec3 sunVelocity = {0.0f, 0.0f, 0.0f};

        void initializeHotkeys();

        void initializeShaders();

        void loadShaders();

        void loadGeometry();

        void initializeScene();

        void bindLighting();

    };
}

#endif //ACAENGINE_ORBITDEMO_H
