#include "orbitdemo.h"

namespace gameState {
    static constexpr auto defaultLightRange = 100.0f;
    static constexpr auto defaultLightIntensity = 0.75f;

    static constexpr auto defaultPlanetPosition = glm::vec3(0.0f, 3.5f, 0.0f);
    static constexpr auto defaultPlanetScale = glm::vec3(1.0f, 1.0f, 1.0f);
    static constexpr auto defaultPlanetVelocity = glm::vec3(-2.0f, -0.5f, 0.0f);

    static constexpr auto defaultSunPosition = glm::vec3(0.0f, -3.5f, 0.0f);
    static constexpr auto defaultSunScale = glm::vec3(1.6f, 1.6f, 1.6f);
    static constexpr auto defaultSunVelocity = glm::vec3(0.5f, 0.125f, 0.0f);

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

    void OrbitDemoState::initializeHotkeys() {
        hotkey_reset_isDown = input::InputManager::isKeyPressed(input::Key::Num1);
        hotkey_mainState_isDown = input::InputManager::isKeyPressed(input::Key::Num2);
        hotkey_exit_isDown = input::InputManager::isKeyPressed(input::Key::Num3);
    }

    void OrbitDemoState::initializeShaders() {
        graphics::Shader::Handle fragmentShader = graphics::ShaderManager::get("shader/demo.frag", graphics::ShaderType::FRAGMENT);
        graphics::Shader::Handle geometryShader = graphics::ShaderManager::get("shader/demo.geom", graphics::ShaderType::GEOMETRY);
        graphics::Shader::Handle vertexShader = graphics::ShaderManager::get("shader/demo.vert", graphics::ShaderType::VERTEX);

        program.attach(vertexShader);
        program.attach(geometryShader);
        program.attach(fragmentShader);
        program.link();
    }

    void OrbitDemoState::loadShaders() {
        program.use();

        cameraControls.loadShaders(program.getID());
        glsl_ambient_light = glGetUniformLocation(program.getID(), "ambient_light");
    }

    void OrbitDemoState::loadGeometry() {
        meshRenderer.clear();

        planetEntity = entity::EntityRegistry::getInstance().createEntity(
                components::Transform(defaultPlanetPosition,
                                      glm::quat(glm::vec3(0.0f, 0.0f, 0.0f)),
                                      defaultPlanetScale),
                components::Mesh(utils::MeshLoader::get("models/sphere.obj"),
                                 graphics::Texture2DManager::get("textures/planet1.png", *graphics::Sampler::getLinearMirroredSampler()),
                                 graphics::Texture2DManager::get("textures/Planet1_phong.png", *graphics::Sampler::getLinearMirroredSampler())),
                components::Velocity(defaultPlanetVelocity),
                components::OrbitalObject(150'000.0)
        );
        meshRenderer.registerMesh(planetEntity);

        const utils::MeshData::Handle sphereData = utils::MeshLoader::get("models/sphere.obj");
        sphereInvertedMeshData = new utils::MeshData;
        sphereInvertedMeshData->positions = sphereData->positions;
        sphereInvertedMeshData->faces = sphereData->faces;
        sphereInvertedMeshData->textureCoordinates = sphereData->textureCoordinates;
        sphereInvertedMeshData->normals = {};
        for (const auto &normal: sphereData->normals) {
            sphereInvertedMeshData->normals.push_back(normal * -1.0f);
        }

        sunEntity = entity::EntityRegistry::getInstance().createEntity(
                components::Transform(defaultSunPosition,
                                      glm::quat(glm::vec3(0.0f, 0.0f, 0.0f)),
                                      defaultSunScale
                ),
                components::Mesh(sphereInvertedMeshData,
                                 graphics::Texture2DManager::get("textures/SunTexture.png", *graphics::Sampler::getLinearMirroredSampler()),
                                 graphics::Texture2DManager::get("textures/Sun_phong.png", *graphics::Sampler::getLinearMirroredSampler())
                ),
                components::Velocity(defaultSunVelocity),
                components::OrbitalObject(600'000.0)
        );
        meshRenderer.registerMesh(sunEntity);

        lightSource = entity::EntityRegistry::getInstance().createEntity(
                components::Light::point(
                        defaultSunPosition, defaultLightRange, glm::vec3(1.0f, 1.0f, 0.8f), defaultLightIntensity
                ),
                components::Transform(defaultSunPosition,
                                      glm::quat(glm::vec3(0.0f, 0.0f, 0.0f)),
                                      glm::vec3(0.5f, 0.5, 0.5f)
                ),
                components::Mesh(sphereInvertedMeshData,
                                 graphics::Texture2DManager::get("textures/SunTexture.png", *graphics::Sampler::getLinearMirroredSampler()),
                                 graphics::Texture2DManager::get("textures/Sun_phong.png", *graphics::Sampler::getLinearMirroredSampler())
                )
        );
        meshRenderer.registerMesh(lightSource);
    }

    void OrbitDemoState::initializeScene() {
        cameraControls.initializeScene();

        entity::EntityRegistry &registry = entity::EntityRegistry::getInstance();

        components::Light light = registry.getComponentData<components::Light>(lightSource).value();
        light.setRange(defaultLightRange);
        light.setIntensity(defaultLightIntensity);
        registry.addOrSetComponent(lightSource, light);

        auto planetTransform = registry.getComponentData<components::Transform>(planetEntity).value();
        planetTransform.setPosition(defaultPlanetPosition);
        registry.addOrSetComponent(planetEntity, planetTransform);

        auto planetVelocity = registry.getComponentData<components::Velocity>(planetEntity).value();
        planetVelocity.velocity = defaultPlanetVelocity;
        registry.addOrSetComponent(planetEntity, planetVelocity);

        auto sunTransform = registry.getComponentData<components::Transform>(sunEntity).value();
        sunTransform.setPosition(defaultSunPosition);
        registry.addOrSetComponent(sunEntity, sunTransform);

        auto sunVelocity = registry.getComponentData<components::Velocity>(sunEntity).value();
        sunVelocity.velocity = defaultSunVelocity;
        registry.addOrSetComponent(sunEntity, sunVelocity);
    }

    void OrbitDemoState::bindLighting() {
        glUniform3fv(glsl_ambient_light, 1, glm::value_ptr(ambientLightData));
        graphics::LightManager::getInstance().bindLights(4);
    }

    OrbitDemoState::OrbitDemoState() :
            cameraControls(graphics::Camera(90.0f, 0.1f, 300.0f), glm::vec3(0.0f, 0.0f, -7.0f), 0.0f, 0.0f, 0.0f),
            ambientLightData({1.4f, 1.4f, 1.4f}),
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

    void OrbitDemoState::update(const long long &deltaMicroseconds) {
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
            onExit();
            return;
        }

        initializeHotkeys();
        cameraControls.update(deltaMicroseconds);

        entity::EntityRegistry &registry = entity::EntityRegistry::getInstance();

        static constexpr float lightRangeStep = 0.1f;
        static constexpr float lightIntensityStep = 0.05f;
        components::Light lightComponent = registry.getComponentData<components::Light>(lightSource).value();
        if (input::InputManager::isKeyPressed(input::Key::R)) {
            lightComponent.setRange(lightComponent.getRange() + lightRangeStep);
            spdlog::info("new range: {}", lightComponent.getRange());
        }
        if (input::InputManager::isKeyPressed(input::Key::F)) {
            lightComponent.setRange(lightComponent.getRange() - lightRangeStep);
            spdlog::info("new range: {}", lightComponent.getRange());
        }
        if (input::InputManager::isKeyPressed(input::Key::T)) {
            lightComponent.setIntensity(lightComponent.getIntensity() + lightIntensityStep);
            spdlog::info("new intensity: {}", lightComponent.getIntensity());
        }
        if (input::InputManager::isKeyPressed(input::Key::G)) {
            lightComponent.setIntensity(lightComponent.getIntensity() - lightIntensityStep);
            spdlog::info("new intensity: {}", lightComponent.getIntensity());
        }

        double deltaSeconds = (double) deltaMicroseconds / 1'000'000.0;
        double deltaSecondsSquared = deltaSeconds * deltaSeconds;
        components::ApplyVelocitySystem(registry, deltaSeconds, deltaSecondsSquared).execute();
        components::OrbitalSystem(registry, deltaSeconds, deltaSecondsSquared).execute();

        components::Transform sunPosition = registry.getComponentData<components::Transform>(sunEntity).value();
        lightComponent.setPosition(sunPosition.getPosition());
        registry.addOrSetComponent(lightSource, lightComponent);
        components::Transform lightTransform = registry.getComponentData<components::Transform>(lightSource).value();
        lightTransform.setPosition(sunPosition.getPosition());
        registry.addOrSetComponent(lightSource, lightTransform);

        meshRenderer.update();
        graphics::LightManager::LightSystem(registry).execute();
    }

    void OrbitDemoState::draw(const long long &deltaMicroseconds) {
        meshRenderer.present(program.getID());
    }

    void OrbitDemoState::onResume() {
        initializeHotkeys();
        cameraControls.initializeCursorPosition();
        loadShaders();
        bindLighting();
        cameraControls.bindCamera();
        printControls();
    }

    void OrbitDemoState::onPause() {
        spdlog::info("===== Orbit Demo State paused =====");
    }

    void OrbitDemoState::onExit() {
        spdlog::info("exiting orbit demo state");

        entity::EntityRegistry::getInstance().eraseEntity(planetEntity);
        entity::EntityRegistry::getInstance().eraseEntity(sunEntity);
        meshRenderer.clear();
        delete planetEntity;
        delete sunEntity;

        delete sphereInvertedMeshData;

        _isFinished = true;
    }
}