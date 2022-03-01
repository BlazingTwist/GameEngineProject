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
#include <engine/utils/containers/octree.hpp>
#include <thread>
#include<engine/entity/componentregistry.h>
#include <iostream>
#include <engine/components/mesh.h>
#include <engine/components/transform.h>
#include <engine/components/bullet.h>
#include <engine/entity/EntityRegistry.h>

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
        graphics::Sampler* sampler = nullptr;
        GLint worldToCameraMatrixID = 0;
        GLint cameraPositionShaderID = 0;
       
        glm::vec3 ambientLightData;
        graphics::LightData lightData;
        graphics::MeshRenderer meshRenderer;
      //  graphics::Mesh sphereMash;
        
    
 
     
        struct Planet
       {
            
            entity::EntityReference* planetID = nullptr;
          // graphics::Mesh sphereMeshes;
           glm::vec3 startposition;
           glm::vec3 direction;
           float velocity;
           float angularVelocityAtStart;
           
           
     
             
      }; 

        

        struct Bullet
        {
            entity::EntityReference* bulletID = nullptr;
            //graphics::Mesh sphereMeshes;
            glm::vec3 bullPos;
            glm::vec3 bullDirection;
            
            
         //   glm::vec3 min;
         //   glm::vec3 max;
            
        };
        std::vector<Planet> planetVec;
        std::vector<Bullet> bulletVec;
    struct TreeProcessor
        {
           int wert = 0;
            std::vector<Planet>fits;
            math::AABB <3, double>& m_bullet;
            const entity::EntityReference* m_entity;
            bool descend(const math::AABB <3, double> &aabb)
            {   
                if (aabb.intersect( m_bullet ))
                    return true;
                else return false;
            }
            void process(const math::AABB <3, double> &aabb, const entity::EntityReference* planet)
            {
                if (aabb.intersect(entity::EntityRegistry::getInstance().getComponentData<components::PhysicsObject>(planet).value()._aabb)) {
                    spdlog::info("collision detected");
                }
                    
             
           
            }
      

            TreeProcessor(math::AABB <3, double> bullet) :
                m_bullet(bullet)
            {}
           

         }; 
     
  
        graphics::Program program = graphics::Program();
        GLint glsl_ambient_light = 0;
        
        bool hotkey_reset_isDown = false;
        bool hotkey_mainState_isDown = false;
        bool hotkey_exit_isDown = false;
        int randomArray[100];
        int randomArray2[100];
        
        void createBullet(const long long int& deltaMicroseconds);
        void updatePos(const long long int& deltaMicroseconds);
        void bindCamera();
        void initializeHotkeys();
        void updateBulletPos(const long long int& deltaMicroseconds);
        void initializeShaders();

        void loadShaders();

        void loadGeometry();
        void createPlanets(const long long& deltaMicroseconds);
        void initializeScene();
        void reloadGeometry();
        void bindLighting();

    };
}
#endif //ACAENGINE_COLLISIONSTATE_H