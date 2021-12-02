#include "freefalldemo.h"

namespace gameState {
    static graphics::Sampler *sampler;
    static constexpr auto defaultSpherePosition = glm::vec3(0.0f, 0.0f, 0.0f);
    static constexpr auto viewportWidth = 4.0f;
    static constexpr auto viewportHeight = 4.0f;
    static constexpr auto viewportPosition = glm::vec2(0.5f, 0.5f);

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

        worldToCameraMatrixID = glGetUniformLocation(program.getID(), "world_to_camera_matrix");
        cameraPositionShaderID = glGetUniformLocation(program.getID(), "camera_position");
        glsl_ambient_light = glGetUniformLocation(program.getID(), "ambient_light");
    }

    void FreeFallDemoState::loadGeometry() {
        meshRenderer.clear();
        mainSphereID = meshRenderer.draw(sphereMesh,
                                         graphics::Texture2DManager::get("textures/planet1.png", *sampler),
                                         graphics::Texture2DManager::get("textures/Planet1_phong.png", *sampler),
                                         glm::translate(glm::identity<glm::mat4>(), glm::vec3(0.0f, 0.0f, 0.0f)));
        transitionSphereID = meshRenderer.draw(sphereMesh,
                                               graphics::Texture2DManager::get("textures/planet1.png", *sampler),
                                               graphics::Texture2DManager::get("textures/Planet1_phong.png", *sampler),
                                               glm::translate(glm::identity<glm::mat4>(), glm::vec3(0.0f, 0.0f, 0.0f)));
    }

    void FreeFallDemoState::initializeScene() {
        spherePosition = defaultSpherePosition;
        updateSpherePosition();
        sphereVelocity = 0.0f;
    }

    void FreeFallDemoState::bindLighting() {
        lightData.bindData(program.getID());
        glUniform3fv(glsl_ambient_light, 1, glm::value_ptr(ambientLightData));
    }

    void FreeFallDemoState::bindCamera() {
        glUniformMatrix4fv(worldToCameraMatrixID, 1, GL_FALSE, glm::value_ptr(camera.getWorldToCamera()));
        glUniform3fv(cameraPositionShaderID, 1, glm::value_ptr(camera.getPosition()));
    }

    FreeFallDemoState::FreeFallDemoState() :
            camera(graphics::Camera(glm::vec2(viewportWidth, viewportHeight), viewportPosition, -10.0f, 20.0f)),
            ambientLightData({0.7f, 0.7f, 0.7f}),
            lightData(graphics::LightData::directional(
                    glm::normalize(glm::vec3(-1.0f, -1.0f, 0.0f)),
                    glm::vec3(1.0f, 1.0f, 1.0f),
                    2.0f
            )),
            meshRenderer(graphics::MeshRenderer()),
            sphereMesh(graphics::Mesh(utils::MeshLoader::get("models/sphere.obj"))) {

        sampler = new graphics::Sampler(graphics::Sampler::Filter::LINEAR, graphics::Sampler::Filter::LINEAR,
                                        graphics::Sampler::Filter::LINEAR, graphics::Sampler::Border::MIRROR);
        
        printControls();
        initializeHotkeys();
        initializeShaders();
        loadShaders();
        loadGeometry();
        initializeScene();
        bindLighting();
        bindCamera();
    }

    void FreeFallDemoState::updateSpherePosition() {
        meshRenderer.setTransform(mainSphereID, glm::translate(glm::identity<glm::mat4>(), spherePosition));
        meshRenderer.setTransform(transitionSphereID, glm::translate(glm::identity<glm::mat4>(), spherePosition + glm::vec3(0.0f, viewportHeight, 0.0f)));
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
        
        static constexpr float sphereAcceleration = 1.0f;
        double deltaSeconds = (double) deltaMicroseconds / 1'000'000.0;
        double deltaSecondsSquared = deltaSeconds * deltaSeconds;
        double velocityGain = sphereAcceleration * deltaSeconds;
        double accelerationDistance = sphereAcceleration * deltaSecondsSquared;
        double totalDistance = (sphereVelocity * deltaSeconds) + accelerationDistance;
        spherePosition += glm::vec3(0.0f, -1.0f, 0.0f) * (float) totalDistance;
        if(spherePosition.y < (-viewportHeight / 2.0f - 1.0f)){
            spherePosition += glm::vec3 (0.0f, viewportHeight, 0.0f);
        }
        sphereVelocity += (float) velocityGain;
        updateSpherePosition();
    }

    void FreeFallDemoState::draw(const long long &deltaMicroseconds) {
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
}