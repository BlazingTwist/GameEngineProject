#include "springdemo.h"

namespace gameState {
    static graphics::Sampler *sampler;
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
        graphics::Shader::Handle vertexShader = graphics::ShaderManager::get("shader/demo.vert", graphics::ShaderType::VERTEX);

        program.attach(vertexShader);
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
        planetID = meshRenderer.draw(sphereMesh,
                                     graphics::Texture2DManager::get("textures/planet1.png", *sampler),
                                     graphics::Texture2DManager::get("textures/Planet1_phong.png", *sampler),
                                     glm::translate(glm::identity<glm::mat4>(), glm::vec3(0.0f, 0.0f, 0.0f)));
        crateID = meshRenderer.draw(crateMesh,
                                    graphics::Texture2DManager::get("textures/cratetex.png", *sampler),
                                    graphics::Texture2DManager::get("textures/Planet1_phong.png", *sampler),
                                    glm::translate(glm::identity<glm::mat4>(), glm::vec3(0.0f, 0.0f, 0.0f)));
    }

    void SpringDemoState::initializeScene() {
        cameraControls.initializeScene();

        lightData.light_position = defaultLightPosition;
        lightData.light_direction = defaultLightDirection;
        lightData.light_spot_angle = defaultLightSpotAngle;

        spherePosition = glm::vec3(-2.0f, 0.0f, 0.0f);
        meshRenderer.setTransform(planetID, glm::translate(glm::identity<glm::mat4>(), spherePosition));
        sphereVelocity = 0.0f;

        auto crateTransform =
                glm::translate(
                        glm::scale(
                                glm::identity<glm::mat4>(),
                                glm::vec3(4.0f, 0.2f, 1.0f)
                        ),
                        glm::vec3(0.0f, -7.0f, 0.0f)
                );
        meshRenderer.setTransform(crateID, crateTransform);
    }

    void SpringDemoState::bindLighting() {
        lightData.bindData(program.getID());
        glUniform3fv(glsl_ambient_light, 1, glm::value_ptr(ambientLightData));
    }

    SpringDemoState::SpringDemoState() :
            cameraControls(graphics::Camera(90.0f, 0.1f, 300.0f), glm::vec3(0.0f, 0.0f, -3.0f), 0.0f, 0.0f, 0.0f),
            ambientLightData({0.7f, 0.7f, 0.7f}),
            lightData(graphics::LightData::spot(
                    defaultLightPosition,
                    defaultLightDirection,
                    25.0f,
                    defaultLightSpotAngle,
                    glm::vec3(1.0f, 1.0f, 0.8f),
                    2.0f
            )),
            meshRenderer(graphics::MeshRenderer()),
            sphereMesh(graphics::Mesh(utils::MeshLoader::get("models/sphere.obj"))),
            crateMesh(graphics::Mesh(utils::MeshLoader::get("models/crate.obj"))) {

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

        static constexpr float spotLightMoveStep = 0.1f;
        static constexpr float spotLightAngleStep = 0.2f;
        bool lightChanged = false;
        if (input::InputManager::isKeyPressed(input::Key::UP)) {
            lightData.light_position += glm::vec3(0.0f, spotLightMoveStep, 0.0f);
            lightChanged = true;
        }
        if (input::InputManager::isKeyPressed(input::Key::DOWN)) {
            lightData.light_position += glm::vec3(0.0f, -spotLightMoveStep, 0.0f);
            lightChanged = true;
        }
        if (input::InputManager::isKeyPressed(input::Key::LEFT)) {
            lightData.light_position += glm::vec3(-spotLightMoveStep, 0.0f, 0.0f);
            lightChanged = true;
        }
        if (input::InputManager::isKeyPressed(input::Key::RIGHT)) {
            lightData.light_position += glm::vec3(spotLightMoveStep, 0.0f, 0.0f);
            lightChanged = true;
        }
        if (input::InputManager::isKeyPressed(input::Key::R)) {
            lightData.light_spot_angle += spotLightAngleStep;
            lightChanged = true;
        }
        if (input::InputManager::isKeyPressed(input::Key::F)) {
            lightData.light_spot_angle -= spotLightAngleStep;
            lightChanged = true;
        }
        if (input::InputManager::isKeyPressed(input::Key::X)) {
            lightData.light_position = cameraControls.camera.getPosition();
            lightData.light_direction = cameraControls.camera.forwardVector();
            lightChanged = true;
        }
        if (lightChanged) {
            bindLighting();
        }

        // Note: this calculation still isn't exactly correct, as the acceleration would change continuously
        //  whereas this approach assumes constant acceleration during every update step
        static constexpr float springConstant = 10.0f;
        static constexpr float sphereMass = 50.0f;
        double deltaSeconds = (double) deltaMicroseconds / 1'000'000.0;
        double deltaSecondsSquared = deltaSeconds * deltaSeconds;
        float xDisplacement = spherePosition.x;
        double displacementForce = 0.0 - (springConstant * xDisplacement);
        double displacementAcceleration = displacementForce / sphereMass; // is in `units / pow(seconds, 2)`
        double displacementVelocity = displacementAcceleration * deltaSeconds;
        double accelerationDistance = displacementAcceleration * deltaSecondsSquared / 2; // extra distance travelled due to gained velocity by accelerating
        double totalTravelDistance = (sphereVelocity * deltaSeconds) + accelerationDistance;
        spherePosition += glm::vec3(1.0f, 0.0f, 0.0f) * (float) totalTravelDistance;
        sphereVelocity += (float) displacementVelocity;
        meshRenderer.setTransform(planetID, glm::translate(glm::identity<glm::mat4>(), spherePosition));
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
}