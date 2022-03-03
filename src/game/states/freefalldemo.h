﻿#ifndef ACAENGINE_FREEFALLDEMO_H
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
#include <iostream>
#include <time.h>
#include <engine/utils/containers/octree.hpp>
#include <thread>
#include<engine/entity/componentregistry.h>
#include <iostream>
#include <engine/components/mesh.h>
#include <engine/components/transform.h>
#include <engine/components/velocity.h>
#include <engine/entity/entityregistry.h>


namespace gameState {
    class FreeFallDemoState : public gameState::BaseGameState {

    public:
        FreeFallDemoState();

        void update(const long long int &deltaMicroseconds) override;

        void draw(const long long int &deltaMicroseconds) override;

        void onResume() override;

        void onPause() override;

    private:
        game::DefaultCameraControls cameraControls;
        graphics::MeshRenderer meshRenderer;

        glm::vec3 ambientLightData;

        std::vector<entity::EntityReference *> planetVec;
        entity::EntityReference *lightSource = nullptr;

        graphics::Program program = graphics::Program();
        GLint glsl_ambient_light = 0;

        double nextPlanetSpawnSeconds = 0.0;
        bool hotkey_exit_isDown = false;

        void initializeHotkeys();

        void initializeShaders();

        void loadShaders();

        void loadGeometry();

        void bindLighting();

        void createObjects(const double &deltaSeconds);

        void onExit();
    };
}

#endif //ACAENGINE_FREEFALLDEMO_H
