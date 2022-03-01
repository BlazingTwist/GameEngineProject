#include "freefalldemo.h"

namespace gameState {
    static constexpr auto defaultLightRange = 100.0f;
    static constexpr auto defaultLightIntensity = 0.75f;
    static float gravConstant = 9.81f;
    static constexpr auto defaultPlanetPosition = glm::vec3(0.0f, 3.5f, 0.0f);
    static constexpr auto defaultPlanetScale = glm::vec3(1.0f, 1.0f, 1.0f);
    static constexpr auto defaultPlanetVelocity = glm::vec3(0.0f, 0.0f, 0.0f);
    double velocityGain = 0;
    static void printControls() {
        spdlog::info("Orbit Demo Controls:");
        spdlog::info("- WASD + EQ = Camera Movement");
        spdlog::info("- Mouse = Camera Yaw/Pitch");
        spdlog::info("- R/F = increase / decrease light range");
        spdlog::info("- T/G = increase / decrease light intensity");
        spdlog::info("- press [1] to reset scene");
        spdlog::info("- press [2] to start a new MainGameState");
        spdlog::info("- press [3] to exit this state");
    }

    void FreeFallDemoState::initializeHotkeys() {
        hotkey_reset_isDown = input::InputManager::isKeyPressed(input::Key::Num1);
        hotkey_mainState_isDown = input::InputManager::isKeyPressed(input::Key::Num2);
        hotkey_exit_isDown = input::InputManager::isKeyPressed(input::Key::Num3);
    }

    void FreeFallDemoState::initializeShaders() {
        graphics::Shader::Handle fragmentShader = graphics::ShaderManager::get("shader/demo.frag", graphics::ShaderType::FRAGMENT);
        graphics::Shader::Handle geometryShader = graphics::ShaderManager::get("shader/demo.geom", graphics::ShaderType::GEOMETRY);
        graphics::Shader::Handle vertexShader = graphics::ShaderManager::get("shader/demo.vert", graphics::ShaderType::VERTEX);

        program.attach(vertexShader);
        program.attach(geometryShader);
        program.attach(fragmentShader);
        program.link();
    }

    void FreeFallDemoState::loadShaders() {
        program.use();

        cameraControls.loadShaders(program.getID());
        glsl_ambient_light = glGetUniformLocation(program.getID(), "ambient_light");
    }

    void FreeFallDemoState::loadGeometry() {
        meshRenderer.clear();

      

       
            lightSource = entity::EntityRegistry::getInstance().createEntity(
                components::Light::directional(
                    glm::vec3(-1.0f, -1.0f, 0.5f), glm::vec3(1.0f, 1.0f, 0.8f), 0.75f
                )
            );
        }

    void FreeFallDemoState::initializeScene() {
      
        cameraControls.initializeScene();


        entity::EntityRegistry& registry = entity::EntityRegistry::getInstance();

       

      //  auto planetTransform = registry.getComponentData<components::Transform>(planetEntity).value();
      //  planetTransform.setPosition(defaultPlanetPosition);
       // registry.addOrSetComponent(planetEntity, planetTransform);

      //  auto planetPhysicsObject = registry.getComponentData<components::PhysicsObject>(planetEntity).value();
       // planetPhysicsObject._velocity = defaultPlanetVelocity;
       // registry.addOrSetComponent(planetEntity, planetPhysicsObject);
    }

    void FreeFallDemoState::bindLighting() {
        glUniform3fv(glsl_ambient_light, 1, glm::value_ptr(ambientLightData));
        graphics::LightManager::getInstance().bindLights(4);
    }

  

    FreeFallDemoState::FreeFallDemoState() :
        cameraControls(graphics::Camera(90.0f, 0.1f, 300.0f), glm::vec3(0.0f, 0.0f, -7.0f), 0.0f, 0.0f, 0.0f),
        ambientLightData({ 1.4f, 1.4f, 1.4f }),
        meshRenderer(graphics::MeshRenderer())
         {

        initializeHotkeys();
        cameraControls.initializeCursorPosition();
        initializeShaders();
        loadShaders();
        loadGeometry();
        cameraControls.initializeScene();
        bindLighting();
        cameraControls.bindCamera();;
        printControls();
    }
    void FreeFallDemoState::createObjects(const double& deltaSeconds)
    {

        nextPlanetSpawnSeconds -= deltaSeconds;
        if (nextPlanetSpawnSeconds <= 0) {
            nextPlanetSpawnSeconds = 2.0;



            std::random_device randomDevice;
            std::mt19937 gen(randomDevice());
            std::uniform_real_distribution<> positionDistribution(0, 9);


            glm::vec3 position = glm::vec3(positionDistribution(gen), 15, positionDistribution(gen));

          entity::EntityReference *planetEntity = entity::EntityRegistry::getInstance().createEntity(
                components::Transform(position,
                    glm::quat(glm::vec3(0.0f, 0.0f, 0.0f)),
                    defaultPlanetScale),
                components::Mesh(utils::MeshLoader::get("models/sphere.obj"),
                    graphics::Texture2DManager::get("textures/planet1.png", *graphics::Sampler::getLinearMirroredSampler()),
                    graphics::Texture2DManager::get("textures/Planet1_phong.png", *graphics::Sampler::getLinearMirroredSampler())),
                components::PhysicsObject(150'000.0, defaultPlanetVelocity)
            );

            meshRenderer.registerMesh(planetEntity);
            planetVec.push_back(planetEntity);
        }
        }


    void FreeFallDemoState::update(const long long &deltaMicroseconds) {
        if (!hotkey_reset_isDown && input::InputManager::isKeyPressed(input::Key::Num1)) {
            spdlog::info("resetting scene");
            initializeScene();
            bindLighting();
        }

        if (!hotkey_mainState_isDown && input::InputManager::isKeyPressed(input::Key::Num2)) {
            spdlog::info("main state hotkey pressed");
            gameState::GameStateManager::getInstance().startGameState(new gameState::MainGameState());
            return;
        }

        if (!hotkey_exit_isDown && input::InputManager::isKeyPressed(input::Key::Num3)) {
            spdlog::info("exiting free fall demo state");
            _isFinished = true;
            return;
        }

        initializeHotkeys();
        if (!hotkey_exit_isDown && input::InputManager::isKeyPressed(input::Key::Num1)) {
            onExit();
            return;
        }
   
        double deltaSeconds = (double) deltaMicroseconds / 1'000'000.0;
        double deltaSecondsSquared = deltaSeconds * deltaSeconds;

     
        
        entity::EntityRegistry::getInstance().execute(
            [deltaSeconds, deltaSecondsSquared,this]( const entity::EntityReference* entity, components::Transform transform,components::PhysicsObject phys)
                 {
         
                             double acc =(9.81)/( deltaSecondsSquared);
                             velocityGain= acc * deltaSeconds;
                             std::cout <<(phys._velocity.y);
                          
                             phys._velocity = phys._velocity - glm::vec3(0, 1, 0) *(float)velocityGain/100000.f;
                           
                            
                            double grenze = transform.getPosition().y;
                            if (grenze > (-20)) {
                                transform.setPosition(transform.getPosition() + phys._velocity);
                                entity::EntityRegistry::getInstance().addOrSetComponent(entity, transform);
                                entity::EntityRegistry::getInstance().addOrSetComponent(entity, phys);
                            }
                               
                            else {
                                std::cout << "hlleo";
                                for (int i=0; i<planetVec.size();i++)
                                {

                                    if (planetVec[i] == entity) {
                                        entity::EntityReference* storedPlanet = planetVec[i];
                                        meshRenderer.removeMesh(storedPlanet);
                                        entity::EntityRegistry::getInstance().eraseEntity(planetVec[i]);
                                        planetVec.erase(planetVec.begin() + i);
                                        return;
                                    }

                                }

                            }
                                
                    });
       
        createObjects(deltaMicroseconds);
        meshRenderer.update();
    }
    
    
    void FreeFallDemoState::draw(const long long& deltaMicroseconds) {
        

        meshRenderer.present(program.getID());
    }
    void FreeFallDemoState::onResume() {
        printControls();
        initializeHotkeys();
        loadShaders();
        bindLighting();
        cameraControls.bindCamera();
    }

    void FreeFallDemoState::onPause() {
        spdlog::info("===== Free Fall Demo State paused =====");
    }
    void FreeFallDemoState::onExit() {
        spdlog::info("exiting orbit demo state");

        entity::EntityRegistry::getInstance().eraseEntity(planetEntity);
        
        delete planetEntity;
        delete sampler;

        _isFinished = true;
    }
}