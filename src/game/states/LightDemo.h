#ifndef ACAENGINE_LIGHTDEMO_H
#define ACAENGINE_LIGHTDEMO_H

#include <vector>
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
#include <engine/components/mesh.h>
#include <engine/components/transform.h>
#include <engine/components/light.h>
#include <engine/graphics/LightManager.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
#include <engine/entity/entityregistry.h>
#include <iostream>
#include <random>

namespace gameState {
    class LightDemo : public gameState::BaseGameState {
    public:
        LightDemo();

        void update(const long long int &deltaMicroseconds) override;

        void draw(const long long int &deltaMicroseconds) override;

        void onResume() override;

        void onPause() override;

    private:
        game::DefaultCameraControls cameraControls;

        glm::vec3 ambientLightData;
        graphics::MeshRenderer meshRenderer;
        utils::MeshData *sphereInvertedMeshData = nullptr;

        entity::EntityReference *xyPlaneEntity = nullptr;
        entity::EntityReference *shipEntity = nullptr;

        graphics::Program program = graphics::Program();
        GLint glsl_ambient_light = 0;

        bool hotkey_spawnLight_isDown = false;
        bool hotkey_destroyLight_isDown = false;
        bool hotkey_exit_isDown = false;

        std::vector<entity::EntityReference *> activeLights = {};

        void initializeHotkeys();

        void initializeShaders();

        void loadShaders();

        void loadGeometry();

        void bindLighting();

        void onExit();
    };
}

#endif //ACAENGINE_LIGHTDEMO_H
