#include "freefalldemo.h"

namespace gameState {
    static constexpr auto defaultLightRange = 100.0f;
    static constexpr auto defaultLightIntensity = 0.75f;

    static constexpr auto defaultPlanetPosition = glm::vec3(0.0f, 3.5f, 0.0f);
    static constexpr auto defaultPlanetScale = glm::vec3(1.0f, 1.0f, 1.0f);
    static constexpr auto defaultPlanetVelocity = glm::vec3(-2.0f, -0.5f, 0.0f);
    static graphics::Sampler* sampler;
    static void printControls() {
        spdlog::info("Free Fall Demo Controls:");
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
        graphics::Shader::Handle vertexShader = graphics::ShaderManager::get("shader/demo.vert", graphics::ShaderType::VERTEX);

        program.attach(vertexShader);
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

        planetEntity = entity::EntityRegistry::getInstance().createEntity(
            components::Transform(defaultPlanetPosition,
                glm::quat(glm::vec3(0.0f, 0.0f, 0.0f)),
                defaultPlanetScale),
            components::Mesh(meshRenderer.requestNewMesh(),
                utils::MeshLoader::get("models/sphere.obj"),
                graphics::Texture2DManager::get("textures/planet1.png", *sampler),
                graphics::Texture2DManager::get("textures/Planet1_phong.png", *sampler)),
            components::PhysicsObject(150'000.0, defaultPlanetVelocity)
        );
    const utils::MeshData::Handle sphereData = utils::MeshLoader::get("models/sphere.obj");
}
    void FreeFallDemoState::initializeScene() {
      
        cameraControls.initializeScene();

        lightData.light_range = defaultLightRange;
        lightData.light_intensity = defaultLightIntensity;

        entity::EntityRegistry& registry = entity::EntityRegistry::getInstance();

        auto planetTransform = registry.getComponentData<components::Transform>(planetEntity).value();
        planetTransform.setPosition(defaultPlanetPosition);
        registry.addOrSetComponent(planetEntity, planetTransform);

        auto planetPhysicsObject = registry.getComponentData<components::PhysicsObject>(planetEntity).value();
        planetPhysicsObject._velocity = defaultPlanetVelocity;
        registry.addOrSetComponent(planetEntity, planetPhysicsObject);
    }

    void FreeFallDemoState::bindLighting() {
        lightData.bindData(program.getID());
        glUniform3fv(glsl_ambient_light, 1, glm::value_ptr(ambientLightData));
    }

  

    FreeFallDemoState::FreeFallDemoState() :
        cameraControls(graphics::Camera(90.0f, 0.1f, 300.0f), glm::vec3(0.0f, 0.0f, -7.0f), 0.0f, 0.0f, 0.0f),
        ambientLightData({ 1.4f, 1.4f, 1.4f }),
        lightData(graphics::LightData::point(
            glm::vec3(0.0f, 0.0f, 0.0f),
            defaultLightRange,
            glm::vec3(1.0f, 1.0f, 0.8f),
            defaultLightIntensity
        )),
        meshRenderer(graphics::MeshRenderer()) {

        sampler = new graphics::Sampler(graphics::Sampler::Filter::LINEAR, graphics::Sampler::Filter::LINEAR,
            graphics::Sampler::Filter::LINEAR, graphics::Sampler::Border::MIRROR);

        printControls();
        initializeHotkeys();
        cameraControls.initializeCursorPosition();
        initializeShaders();
        loadShaders();
        loadGeometry();
        initializeScene();
        bindLighting();
        cameraControls.bindCamera();
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

        static constexpr float lightRangeStep = 0.1f;
        static constexpr float lightIntensityStep = 0.05f;
        if (input::InputManager::isKeyPressed(input::Key::R)) {
            lightData.light_range += lightRangeStep;
        }
        if (input::InputManager::isKeyPressed(input::Key::F)) {
            lightData.light_range -= lightRangeStep;
        }
        if (input::InputManager::isKeyPressed(input::Key::T)) {
            lightData.light_intensity += lightIntensityStep;
        }
        if (input::InputManager::isKeyPressed(input::Key::G)) {
            lightData.light_intensity -= lightIntensityStep;
        }
        ;
        static float gravConstant = 9.81f;
        double deltaSeconds = (double) deltaMicroseconds / 1'000'000.0;
        double deltaSecondsSquared = deltaSeconds * deltaSeconds;
       // double velocityGain =gravConstant * deltaSeconds;

      //  double accelerationDistance = sphereAcceleration * deltaSecondsSquared;
        //double totalDistance = (sphereVelocity * deltaSeconds) + accelerationDistance;
     
     
        
        entity::EntityRegistry::getInstance().execute(
            [deltaSeconds, deltaSecondsSquared](const entity::EntityReference* entity2, components::Transform transform2, components::PhysicsObject phys2) {
                entity::EntityRegistry::getInstance().execute(
                    [entity2, transform2, phys2, deltaSeconds, deltaSecondsSquared](const entity::EntityReference* entity,
                        components::Transform transform,
                        components::PhysicsObject phys) {
                            if (entity->getReferenceID() == entity2->getReferenceID()) {
                                return;
                            }
                            
                            glm::vec3 aToB = transform2.getPosition() - transform.getPosition();
                            glm::vec3 aToBNormal = glm::normalize(aToB);
                            float distanceSquared = glm::dot(aToB, aToB);
                            double acceleration = gravConstant;
                            double velocityGain = acceleration * deltaSeconds;
                            double accelerationDistance = acceleration * deltaSecondsSquared / 2;
                            transform.setPosition(
                                transform.getPosition() + (phys._velocity * (float)deltaSeconds) + (aToBNormal * (float)accelerationDistance));
                            phys._velocity = phys._velocity + (aToBNormal * (float)velocityGain);
                            entity::EntityRegistry::getInstance().addOrSetComponent(entity, transform);
                            entity::EntityRegistry::getInstance().addOrSetComponent(entity, phys);
                    });
            });

        components::Transform planetPosition = entity::EntityRegistry::getInstance().getComponentData<components::Transform>(planetEntity).value();
        lightData.light_position = planetPosition.getPosition();
        bindLighting();
    }
    

    void OrbitDemoState::draw(const long long& deltaMicroseconds) {
        auto& registry = entity::EntityRegistry::getInstance();
        registry.execute([this, &registry](const entity::EntityReference* entity, components::Mesh mesh, components::Transform transform) {
            bool meshChanged = mesh.hasAnyChanges();
            bool transformChanged = transform.hasTransformChanged();
            meshRenderer.draw(mesh, transform);
            if (meshChanged) {
                registry.addOrSetComponent(entity, mesh);
            }
            if (transformChanged) {
                registry.addOrSetComponent(entity, transform);
            }
            });

        meshRenderer.present(program.getID());
    }
    void FreeFallDemoState::onResume() {
        printControls();
        initializeHotkeys();
        loadShaders();
        bindLighting();
        bindCamera();
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