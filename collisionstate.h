#ifndef ACAENGINE_COLLISIONSTATE_H
#define ACAENGINE_COLLISIONSTATE_H
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

namespace gameState {
    class CollisionState : public gameState::BaseGameState {

    public:
        CollisionState();

        void update(const long long int& deltaMicroseconds) override;

        void draw(const long long int& deltaMicroseconds) override;

        void onResume() override;

        void onPause() override;

    private:
        game::DefaultCameraControls cameraControls;
       
        GLint worldToCameraMatrixID = 0;
        GLint cameraPositionShaderID = 0;

        glm::vec3 ambientLightData;
        graphics::LightData lightData;
        graphics::MeshRenderer meshRenderer;
        graphics::Mesh sphereMash;
      
    
        struct AABB
        {
            glm::vec3 min;
            glm::vec3 max;
        };
    

        struct Planet
       {
            
           unsigned int planetID;
           graphics::Mesh sphereMeshes;
           glm::vec3 startposition;
           glm::vec3 direction;
           float velocity;
           //part of collision
           //glm::vec3 min;
          //glm::vec3 max;
           float angularVelocity;
            
           
     
            AABB aabb;
      }; 

        struct Bullet
        {
            unsigned int bulletID;
            graphics::Mesh sphereMeshes;
            glm::vec3 bullPos;
            glm::vec3 bullDirection;
            float velocity;
         //   glm::vec3 min;
         //   glm::vec3 max;
            
        };

        std::vector<Planet> planetVec;
        std::vector<Bullet> bulletVec;
        graphics::Program program = graphics::Program();
        GLint glsl_ambient_light = 0;
        
        bool hotkey_reset_isDown = false;
        bool hotkey_mainState_isDown = false;
        bool hotkey_exit_isDown = false;
        int randomArray[50];
        int randomArray2[50];

        void createBullet(const long long int& deltaMicroseconds);
        void updatePos(const long long int& deltaMicroseconds);
        void bindCamera();
        void initializeHotkeys();
        void updateBulletPos(const long long int& deltaMicroseconds);
        void initializeShaders();

        void loadShaders();

        void loadGeometry();

        void initializeScene();

        void bindLighting();

    };
}
#endif //ACAENGINE_COLLISIONSTATE_H