#include "springdemo.h"

namespace gameState {
    static constexpr auto defaultPlanetPosition = glm::vec3(-2.0f, 0.0f, 0.0f);

    static constexpr auto defaultLightPosition = glm::vec3(0.0f, 0.0f, -3.0f);
    static constexpr auto defaultLightDirection = glm::vec3(0.0f, 0.0f, 1.0f);
    static constexpr auto defaultLightSpotAngle = 25.0f;

    static void printControls() {
        spdlog::info("Spring Demo Controls:");
        spdlog::info("- WASD + EQ = Camera Movement");
        spdlog::info("- Mouse = Camera Yaw/Pitch");
        spdlog::info("- Arrows = Move light-source in x/y plane");
        spdlog::info("- R/F = increase / decrease light spot-angle");
        spdlog::info("- X = move spotlight to camera position and direction");
        spdlog::info("- press [1] to reset scene");
        spdlog::info("- press [2] to start a new MainGameState");
        spdlog::info("- press [3] to exit this state");
    }

    void SpringDemoState::initializeHotkeys() {
        hotkey_reset_isDown = input::InputManager::isKeyPressed(input::Key::Num1);
        hotkey_mainState_isDown = input::InputManager::isKeyPressed(input::Key::Num2);
        hotkey_exit_isDown = input::InputManager::isKeyPressed(input::Key::Num3);
    }

    void SpringDemoState::initializeShaders() {
        graphics::Shader::Handle fragmentShader = graphics::ShaderManager::get("shader/demo.frag", graphics::ShaderType::FRAGMENT);
        graphics::Shader::Handle geometryShader = graphics::ShaderManager::get("shader/demo.geom", graphics::ShaderType::GEOMETRY);
        graphics::Shader::Handle vertexShader = graphics::ShaderManager::get("shader/demo.vert", graphics::ShaderType::VERTEX);

        program.attach(vertexShader);
        program.attach(geometryShader);
        program.attach(fragmentShader);
        program.link();
    }

    void SpringDemoState::loadShaders() {
        program.use();

        cameraControls.loadShaders(program.getID());
        glsl_ambient_light = glGetUniformLocation(program.getID(), "ambient_light");
    }

    void SpringDemoState::loadGeometry() {
        meshRenderer.clear();

        planetEntity = entity::EntityRegistry::getInstance().createEntity(
                components::Mesh(utils::MeshLoader::get("models/sphere.obj"),
                                 graphics::Texture2DManager::get("textures/planet1.png", *graphics::Sampler::getLinearMirroredSampler()),
                                 graphics::Texture2DManager::get("textures/Planet1_phong.png", *graphics::Sampler::getLinearMirroredSampler())
                ),
                components::Transform(defaultPlanetPosition,
                                      glm::quat(glm::vec3(0.0f, 0.0f, 0.0f)),
                                      glm::vec3(1.0f, 1.0f, 1.0f)
                )
        );
        meshRenderer.registerMesh(planetEntity);

        crateEntity = entity::EntityRegistry::getInstance().createEntity(
                components::Mesh(utils::MeshLoader::get("models/crate.obj"),
                                 graphics::Texture2DManager::get("textures/cratetex.png", *graphics::Sampler::getLinearMirroredSampler())
                ),
                components::Transform(glm::vec3(0.0f, -1.5f, 0.0f),
                                      glm::quat(glm::vec3(0.0f, 0.0f, 0.0f)),
                                      glm::vec3(4.0f, 0.2f, 1.0f)
                )
        );
        meshRenderer.registerMesh(crateEntity);

        lightSource = entity::EntityRegistry::getInstance().createEntity(
                components::Light::spot(
                        defaultLightPosition,
                        defaultLightDirection,
                        25.0f,
                        defaultLightSpotAngle,
                        glm::vec3(1.0f, 1.0f, 0.8f),
                        2.0f
                )
        );
    }

    void SpringDemoState::initializeScene() {
        cameraControls.initializeScene();
        entity::EntityRegistry &registry = entity::EntityRegistry::getInstance();

        components::Light light = registry.getComponentData<components::Light>(lightSource).value();
        light.setPosition(defaultLightPosition);
        light.setDirection(defaultLightDirection);
        light.setSpotAngle(defaultLightSpotAngle);
        registry.addOrSetComponent(lightSource, light);

        components::Transform planetTransform = registry.getComponentData<components::Transform>(planetEntity).value();
        planetTransform.setPosition(defaultPlanetPosition);
        registry.addOrSetComponent(planetEntity, planetTransform);

        planetVelocity = 0.0f;
    }

    void SpringDemoState::bindLighting() {
        glUniform3fv(glsl_ambient_light, 1, glm::value_ptr(ambientLightData));
        graphics::LightManager::getInstance().bindLights(4);
    }

    SpringDemoState::SpringDemoState() :
            cameraControls(graphics::Camera(90.0f, 0.1f, 300.0f), glm::vec3(0.0f, 0.0f, -3.0f), 0.0f, 0.0f, 0.0f),
            ambientLightData({0.7f, 0.7f, 0.7f}),
            meshRenderer(graphics::MeshRenderer()) {

        initializeHotkeys();
        cameraControls.initializeCursorPosition();
        initializeShaders();
        loadShaders();
        loadGeometry();
        initializeScene();
        bindLighting();
        cameraControls.bindCamera();

        printControls();
    }

    void SpringDemoState::update(const long long &deltaMicroseconds) {
        if (!hotkey_reset_isDown && input::InputManager::isKeyPressed(input::Key::Num1)) {
            spdlog::info("resetting scene");
            initializeScene();
            bindLighting();
            cameraControls.bindCamera();
        }

        if (!hotkey_mainState_isDown && input::InputManager::isKeyPressed(input::Key::Num2)) {
            spdlog::info("main state hotkey pressed");
            gameState::GameStateManager::getInstance().startGameState(new gameState::MainGameState());
            return;
        }

        if (!hotkey_exit_isDown && input::InputManager::isKeyPressed(input::Key::Num3)) {
            spdlog::info("exiting spring demo state");
            _isFinished = true;
            return;
        }

        initializeHotkeys();
        cameraControls.update(deltaMicroseconds);

        entity::EntityRegistry &registry = entity::EntityRegistry::getInstance();

        static constexpr float spotLightMoveStep = 0.1f;
        static constexpr float spotLightAngleStep = 0.2f;
        components::Light lightComponent = registry.getComponentData<components::Light>(lightSource).value();
        if (input::InputManager::isKeyPressed(input::Key::UP)) {
            lightComponent.setPosition(lightComponent.getPosition() + glm::vec3(0.0f, spotLightMoveStep, 0.0f));
        }
        if (input::InputManager::isKeyPressed(input::Key::DOWN)) {
            lightComponent.setPosition(lightComponent.getPosition() + glm::vec3(0.0f, -spotLightMoveStep, 0.0f));
        }
        if (input::InputManager::isKeyPressed(input::Key::LEFT)) {
            lightComponent.setPosition(lightComponent.getPosition() + glm::vec3(-spotLightMoveStep, 0.0f, 0.0f));
        }
        if (input::InputManager::isKeyPressed(input::Key::RIGHT)) {
            lightComponent.setPosition(lightComponent.getPosition() + glm::vec3(spotLightMoveStep, 0.0f, 0.0f));
        }
        if (input::InputManager::isKeyPressed(input::Key::R)) {
            lightComponent.setSpotAngle(lightComponent.getSpotAngle() + spotLightAngleStep);
        }
        if (input::InputManager::isKeyPressed(input::Key::F)) {
            lightComponent.setSpotAngle(lightComponent.getSpotAngle() - spotLightAngleStep);
        }
        if (input::InputManager::isKeyPressed(input::Key::X)) {
            lightComponent.setPosition(cameraControls.camera.getPosition());
            lightComponent.setDirection(cameraControls.camera.forwardVector());
        }
        registry.addOrSetComponent(lightSource, lightComponent);

        // Note: this calculation still isn't exactly correct, as the acceleration would change continuously
        //  whereas this approach assumes constant acceleration during every update step
        components::Transform planetTransform = registry.getComponentData<components::Transform>(planetEntity).value();
        glm::vec3 planetPosition = planetTransform.getPosition();
        static constexpr float springConstant = 10.0f;
        static constexpr float sphereMass = 50.0f;
        double deltaSeconds = (double) deltaMicroseconds / 1'000'000.0;
        double deltaSecondsSquared = deltaSeconds * deltaSeconds;
        float xDisplacement = planetPosition.x;
        double displacementForce = 0.0 - (springConstant * xDisplacement);
        double displacementAcceleration = displacementForce / sphereMass; // is in `units / pow(seconds, 2)`
        double displacementVelocity = displacementAcceleration * deltaSeconds;
        double accelerationDistance = displacementAcceleration * deltaSecondsSquared / 2; // extra distance travelled due to gained velocity by accelerating
        double totalTravelDistance = (planetVelocity * deltaSeconds) + accelerationDistance;
        planetPosition += glm::vec3(1.0f, 0.0f, 0.0f) * (float) totalTravelDistance;
        planetVelocity += (float) displacementVelocity;
        planetTransform.setPosition(planetPosition);
        registry.addOrSetComponent(planetEntity, planetTransform);

        meshRenderer.update();
        graphics::LightManager::LightSystem(registry).execute();
    }

    void SpringDemoState::draw(const long long &deltaMicroseconds) {
        meshRenderer.present(program.getID());
    }

    void SpringDemoState::onResume() {
        printControls();
        initializeHotkeys();
        cameraControls.initializeCursorPosition();
        loadShaders();
        bindLighting();
        cameraControls.bindCamera();
    }

    void SpringDemoState::onPause() {
        spdlog::info("===== Spring Demo State paused =====");
    }

    void SpringDemoState::onExit() {
        spdlog::info("exiting Spring Demo state");

        meshRenderer.clear();
        graphics::LightManager::getInstance().removeLight(lightSource);

        entity::EntityRegistry::getInstance().eraseEntity(planetEntity);
        entity::EntityRegistry::getInstance().eraseEntity(crateEntity);
        entity::EntityRegistry::getInstance().eraseEntity(lightSource);

        delete planetEntity;
        delete crateEntity;
        delete lightSource;

        _isFinished = true;
    }
}