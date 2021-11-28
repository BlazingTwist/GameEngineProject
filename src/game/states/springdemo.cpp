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

    void SpringDemoState::initializeControls() {
        hotkey_reset_isDown = input::InputManager::isKeyPressed(input::Key::Num1);
        hotkey_mainState_isDown = input::InputManager::isKeyPressed(input::Key::Num2);
        hotkey_exit_isDown = input::InputManager::isKeyPressed(input::Key::Num3);
    }

    void SpringDemoState::initializeCursorPosition() {
        prevCursorPos = input::InputManager::getCursorPos();
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

        worldToCameraMatrixID = glGetUniformLocation(program.getID(), "world_to_camera_matrix");
        cameraPositionShaderID = glGetUniformLocation(program.getID(), "camera_position");
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
        camera.setPosition(glm::vec3(0.0f, 0.0f, -3.0f));
        cameraPitch = 0.0f;
        cameraYaw = 0.0f;
        cameraRoll = 0.0f;
        camera.setRotation(cameraYaw, cameraPitch, cameraRoll);

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

    void SpringDemoState::bindCamera() {
        glUniformMatrix4fv(worldToCameraMatrixID, 1, GL_FALSE, glm::value_ptr(camera.getWorldToCamera()));
        glUniform3fv(cameraPositionShaderID, 1, glm::value_ptr(camera.getPosition()));
    }

    SpringDemoState::SpringDemoState() :
            ambientLightData({0.7f, 0.7f, 0.7f}),
            lightData(graphics::LightData::spot(
                    defaultLightPosition,
                    defaultLightDirection,
                    25.0f,
                    defaultLightSpotAngle,
                    glm::vec3(1.0f, 1.0f, 0.8f),
                    2.0f
            )),
            camera(graphics::Camera(90.0f, 0.1f, 300.0f)),
            meshRenderer(graphics::MeshRenderer()),
            sphereMesh(graphics::Mesh(utils::MeshLoader::get("models/sphere.obj"))),
            crateMesh(graphics::Mesh(utils::MeshLoader::get("models/crate.obj"))) {

        sampler = new graphics::Sampler(graphics::Sampler::Filter::LINEAR, graphics::Sampler::Filter::LINEAR,
                                        graphics::Sampler::Filter::LINEAR, graphics::Sampler::Border::MIRROR);

        printControls();
        initializeControls();
        initializeCursorPosition();
        initializeShaders();
        loadShaders();
        loadGeometry();
        initializeScene();
        bindLighting();
        bindCamera();
    }

    void SpringDemoState::update(const long long &deltaMicroseconds) {
        if (!hotkey_reset_isDown && input::InputManager::isKeyPressed(input::Key::Num1)) {
            spdlog::info("resetting scene");
            initializeScene();
            bindLighting();
            bindCamera();
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

        initializeControls();

        static constexpr float cameraStep = 0.1f;
        static constexpr float cameraPitchSensitivity = 0.5f;
        static constexpr float cameraYawSensitivity = 0.5f;
        float rightInput = 0.0f;
        float forwardInput = 0.0f;
        float upInput = 0.0f;
        bool cameraPositionChanged = false;
        if (input::InputManager::isKeyPressed(input::Key::W)) {
            cameraPositionChanged = true;
            forwardInput += cameraStep;
        }
        if (input::InputManager::isKeyPressed(input::Key::S)) {
            cameraPositionChanged = true;
            forwardInput -= cameraStep;
        }
        if (input::InputManager::isKeyPressed(input::Key::D)) {
            cameraPositionChanged = true;
            rightInput += cameraStep;
        }
        if (input::InputManager::isKeyPressed(input::Key::A)) {
            cameraPositionChanged = true;
            rightInput -= cameraStep;
        }
        if (input::InputManager::isKeyPressed(input::Key::E)) {
            cameraPositionChanged = true;
            upInput += cameraStep;
        }
        if (input::InputManager::isKeyPressed(input::Key::Q)) {
            cameraPositionChanged = true;
            upInput -= cameraStep;
        }
        if (cameraPositionChanged) {
            camera.moveRelative(rightInput, upInput, forwardInput);
        }

        glm::vec2 currentCursorPos = input::InputManager::getCursorPos();
        float cursorDeltaX = currentCursorPos.x - prevCursorPos.x;
        float cursorDeltaY = currentCursorPos.y - prevCursorPos.y;
        prevCursorPos = currentCursorPos;
        bool cameraDirectionChanged = false;
        if (cursorDeltaX != 0.0f) {
            cameraDirectionChanged = true;
            cameraYaw += cursorDeltaX * cameraYawSensitivity;
            if (cameraYaw > 180.0f) {
                cameraYaw -= 360.0f;
            } else if (cameraYaw < -180.0f) {
                cameraYaw += 360.0f;
            }
        }
        if (cursorDeltaY != 0.0f) {
            cameraDirectionChanged = true;
            cameraPitch += cursorDeltaY * cameraPitchSensitivity;
            if (cameraPitch > 90.0f) {
                cameraPitch = 90.0f;
            } else if (cameraPitch < -90.0f) {
                cameraPitch = -90.0f;
            }
        }
        if (cameraDirectionChanged) {
            camera.setRotation(cameraYaw, cameraPitch, cameraRoll);
        }

        if (cameraPositionChanged || cameraDirectionChanged) {
            bindCamera();
        }

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
            lightData.light_position = camera.getPosition();
            lightData.light_direction = camera.forwardVector();
            lightChanged = true;
        }
        if (lightChanged) {
            bindLighting();
        }

        static constexpr float springConstant = 10.0f;
        static constexpr float sphereMass = 50.0f;
        float xDisplacement = spherePosition.x;
        double displacementForce = 0.0 - (springConstant * xDisplacement);
        double displacementAcceleration = displacementForce / sphereMass; // is in `units / pow(seconds, 2)`
        double displacementVelocity = displacementAcceleration * (double) deltaMicroseconds / 1'000'000.0;
        sphereVelocity += (float) displacementVelocity;
        spherePosition += glm::vec3(1.0f, 0.0f, 0.0f) * sphereVelocity;
        meshRenderer.setTransform(planetID, glm::translate(glm::identity<glm::mat4>(), spherePosition));
    }

    void SpringDemoState::draw(const long long &deltaMicroseconds) {
        meshRenderer.present(program.getID());
    }

    void SpringDemoState::onResume() {
        printControls();
        initializeControls();
        initializeCursorPosition();
        loadShaders();
        bindLighting();
        bindCamera();
    }

    void SpringDemoState::onPause() {
        spdlog::info("===== Spring Demo State paused =====");
    }
}