#ifndef ACAENGINE_SPACESIM_H
#define ACAENGINE_SPACESIM_H

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
#include <engine/components/mesh.h>
#include <engine/components/transform.h>
#include <engine/components/light.h>
#include <engine/components/velocity.h>
#include <engine/components/OrbitalObject.h>
#include <engine/components/RotationalVelocity.h>
#include <engine/components/ScaleVelocity.h>
#include <engine/graphics/LightManager.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
#include <engine/entity/entityregistry.h>
#include <game/camera/followcamera.h>

namespace gameState {
    class SpaceSim : public gameState::BaseGameState {

    private:
        struct ProjectileData {
            ProjectileData(entity::EntityReference *projectileEntity, double remainingLifeTime)
                    : projectileEntity(projectileEntity), remainingLifeTime(remainingLifeTime) {}

            entity::EntityReference *projectileEntity;
            double remainingLifeTime;
        };

    public:
        SpaceSim();

        void update(const long long int &deltaMicroseconds) override;

        void draw(const long long int &deltaMicroseconds) override;

        void onResume() override;

        void onPause() override;

    private:
        glm::vec3 ambientLightData;
        graphics::MeshRenderer meshRenderer;

        std::vector<entity::EntityReference *> solarSystemEntities = {};
        entity::EntityReference *playerShipEntity = nullptr;
        std::vector<ProjectileData> activeProjectiles = {};
        entity::EntityReference *skyboxEntity = nullptr;

        graphics::Program program = graphics::Program();
        GLint glsl_ambient_light = 0;

        bool hotkey_camera1_isDown = false;
        bool hotkey_camera2_isDown = false;
        bool hotkey_camera3_isDown = false;
        bool hotkey_cameraReset_isDown = false;
        bool hotkey_exit_isDown = false;

        int currentPlayerThrottle = 0;
        bool throttleKeyIsDown = false;
        glm::vec3 cannonOffsets[4]{
                glm::vec3(-6.6f, 0.8f, 0.58f), // top left (outermost)
                glm::vec3(-2.4f, -0.6f, -0.45f), // middle left
                glm::vec3(4.5f, -1.2f, -0.4f), // bottom left (second to outermost)
                glm::vec3(-0.7f, -0.45f, -4.9f), // center left
        };
        int currentCannonIndex = 0;
        double remainingCannonCooldown = 0;

        graphics::Camera cameraInstance;
        game::FollowCamera *activeFollowCamera;
        game::FollowCamera camera1;
        game::FollowCamera camera2;
        game::FollowCamera camera3;

        utils::MeshData *sphereInvertedMeshData = nullptr;

        void initializeHotkeys();

        void checkHotkeys(const long long int &deltaMicroseconds);

        void initializeShaders();

        void loadShaders();

        void prepareEntities();

        void bindLighting();

        void handleFlightControls(double deltaSeconds, double deltaSecondsSquared);

        void spawnProjectile(const components::Transform &shipTransform, const glm::vec3 &spawnOffset, const glm::vec3 &velocity);

        void onExit();

    };
}

#endif //ACAENGINE_SPACESIM_H