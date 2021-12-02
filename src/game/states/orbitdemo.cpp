#include "orbitdemo.h"

namespace gameState {
    static graphics::Sampler *sampler;
    static constexpr auto defaultLightRange = 100.0f;
    static constexpr auto defaultLightIntensity = 0.75f;
    static constexpr auto defaultPlanetPosition = glm::vec3(0.0f, 3.5f, 0.0f);
    static constexpr auto defaultPlanetVelocity = glm::vec3(-2.0f, -0.5f, 0.0f);
    static constexpr auto defaultSunPosition = glm::vec3(0.0f, -3.5f, 0.0f);
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
        graphics::Shader::Handle vertexShader = graphics::ShaderManager::get("shader/demo.vert", graphics::ShaderType::VERTEX);

        program.attach(vertexShader);
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
        planetID = meshRenderer.draw(sphereMesh,
                                     graphics::Texture2DManager::get("textures/planet1.png", *sampler),
                                     graphics::Texture2DManager::get("textures/Planet1_phong.png", *sampler),
                                     glm::translate(glm::identity<glm::mat4>(), glm::vec3(0.0f, 0.0f, 0.0f)));
        if (sphereInvertedMesh != nullptr) {
            sunID = meshRenderer.draw(*sphereInvertedMesh,
                                      graphics::Texture2DManager::get("textures/SunTexture.png", *sampler),
                                      graphics::Texture2DManager::get("textures/Planet1_phong.png", *sampler),
                                      glm::translate(glm::identity<glm::mat4>(), glm::vec3(0.0f, 0.0f, 0.0f)));
        } else {
            spdlog::error("OrbitDemo - sphere inverted mesh was not initialized!");
        }
    }

    void OrbitDemoState::initializeScene() {
        cameraControls.initializeScene();

        lightData.light_range = defaultLightRange;
        lightData.light_intensity = defaultLightIntensity;

        planetPosition = defaultPlanetPosition;
        meshRenderer.setTransform(planetID, glm::translate(glm::identity<glm::mat4>(), defaultPlanetPosition));
        planetVelocity = defaultPlanetVelocity;

        sunPosition = defaultSunPosition;
        meshRenderer.setTransform(sunID, glm::translate(glm::identity<glm::mat4>(), defaultSunPosition));
        sunVelocity = defaultSunVelocity;
    }

    void OrbitDemoState::bindLighting() {
        lightData.bindData(program.getID());
        glUniform3fv(glsl_ambient_light, 1, glm::value_ptr(ambientLightData));
    }

    OrbitDemoState::OrbitDemoState() :
            cameraControls(graphics::Camera(90.0f, 0.1f, 300.0f), glm::vec3(0.0f, 0.0f, -7.0f), 0.0f, 0.0f, 0.0f),
            ambientLightData({1.4f, 1.4f, 1.4f}),
            lightData(graphics::LightData::point(
                    glm::vec3(0.0f, 0.0f, 0.0f),
                    defaultLightRange,
                    glm::vec3(1.0f, 1.0f, 0.8f),
                    defaultLightIntensity
            )),
            meshRenderer(graphics::MeshRenderer()),
            sphereMesh(graphics::Mesh(utils::MeshLoader::get("models/sphere.obj"))) {

        const utils::MeshData::Handle sphereData = utils::MeshLoader::get("models/sphere.obj");
        sphereInvertedMeshData = new utils::MeshData;
        sphereInvertedMeshData->positions = sphereData->positions;
        sphereInvertedMeshData->faces = sphereData->faces;
        sphereInvertedMeshData->textureCoordinates = sphereData->textureCoordinates;
        sphereInvertedMeshData->normals = {};
        for (const auto &normal: sphereData->normals) {
            sphereInvertedMeshData->normals.push_back(normal * -1.0f);
        }
        sphereInvertedMesh = new graphics::Mesh(sphereInvertedMeshData);

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
            spdlog::info("exiting orbit demo state");
            _isFinished = true;
            return;
        }

        initializeHotkeys();
        cameraControls.update(deltaMicroseconds);

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

        static constexpr double earthMass = 150'000.0;
        static constexpr double sunMass = 600'000.0;
        static constexpr double gravConstant = 6.6743e-5;
        double deltaSeconds = (double) deltaMicroseconds / 1'000'000.0;
        double deltaSecondsSquared = deltaSeconds * deltaSeconds;
        glm::vec3 sunToPlanet = planetPosition - sunPosition;
        glm::vec3 sunToPlanetNormal = glm::normalize(sunToPlanet);
        float distanceSquared = glm::dot(sunToPlanet, sunToPlanet);
        double gravForce = gravConstant * earthMass * sunMass / distanceSquared;
        {
            // Move Sun
            double acceleration = gravForce / sunMass;
            double velocityGain = acceleration * deltaSeconds;
            double accelerationDistance = acceleration * deltaSecondsSquared / 2;
            sunPosition = sunPosition + (sunVelocity * (float) deltaSeconds) + (sunToPlanetNormal * (float) accelerationDistance);
            sunVelocity = sunVelocity + (sunToPlanetNormal * (float) velocityGain);
            meshRenderer.setTransform(sunID, glm::translate(glm::identity<glm::mat4>(), sunPosition));

            lightData.light_position = sunPosition;
            bindLighting();
        }
        {
            // Move Planet
            double acceleration = gravForce / earthMass;
            double velocityGain = acceleration * deltaSeconds;
            double accelerationDistance = acceleration * deltaSecondsSquared / 2;
            planetPosition = planetPosition + (planetVelocity * (float) deltaSeconds) + (-sunToPlanetNormal * (float) accelerationDistance);
            planetVelocity = planetVelocity + (-sunToPlanetNormal * (float) velocityGain);
            meshRenderer.setTransform(planetID, glm::translate(glm::identity<glm::mat4>(), planetPosition));
        }
    }

    void OrbitDemoState::draw(const long long &deltaMicroseconds) {
        meshRenderer.present(program.getID());
    }

    void OrbitDemoState::onResume() {
        printControls();
        initializeHotkeys();
        cameraControls.initializeCursorPosition();
        loadShaders();
        bindLighting();
        cameraControls.bindCamera();
    }

    void OrbitDemoState::onPause() {
        spdlog::info("===== Orbit Demo State paused =====");
    }
}