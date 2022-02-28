#ifndef ACAENGINE_PARALLAXOCCLUSIONDEMO_H
#define ACAENGINE_PARALLAXOCCLUSIONDEMO_H

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
#include "mainstate.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
#include <engine/entity/entityregistry.h>

namespace gameState {
    class ParallaxOcclusionDemo : public gameState::BaseGameState {

    public:
        ParallaxOcclusionDemo();

        void update(const long long int &deltaMicroseconds) override;

        void draw(const long long int &deltaMicroseconds) override;

        void onResume() override;

        void onPause() override;

    private:
        game::DefaultCameraControls cameraControls;

        glm::vec3 ambientLightData;
        graphics::LightData lightData;
        graphics::MeshRenderer meshRenderer;
        
        entity::EntityReference* xyPlaneEntity = nullptr;

        graphics::Program program = graphics::Program();
        GLint glsl_ambient_light = 0;
        
        bool hotkey_toggleTexture_isDown = false;
        bool hotkey_togglePhong_isDown = false;
        bool hotkey_toggleNormal_isDown = false;
        bool hotkey_toggleHeightMap_isDown = false;
        bool hotkey_exit_isDown = false;

        void initializeHotkeys();

        void initializeShaders();

        void loadShaders();

        void loadGeometry();
        
        void bindLighting();

        void onExit();

    };
}

#endif //ACAENGINE_PARALLAXOCCLUSIONDEMO_H
