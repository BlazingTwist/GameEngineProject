#include "ParallaxOcclusionDemo.h"

namespace gameState {

    const graphics::Texture2D *getWoodTexture() {
        static auto defaultTexture = graphics::Texture2DManager::get("textures/woodlogwall/texture.png", *graphics::Sampler::getLinearMirroredSampler());
        return defaultTexture;
    }

    const graphics::Texture2D *getWoodPhong() {
        static auto defaultTexture = graphics::Texture2DManager::get("textures/woodlogwall/phong.png", *graphics::Sampler::getLinearMirroredSampler());
        return defaultTexture;
    }

    const graphics::Texture2D *getWoodNormal() {
        static auto defaultTexture = graphics::Texture2DManager::get("textures/woodlogwall/normal.png", *graphics::Sampler::getLinearMirroredSampler());
        return defaultTexture;
    }

    const graphics::Texture2D *getWoodHeightMap() {
        static auto defaultTexture = graphics::Texture2DManager::get("textures/woodlogwall/heightmap.png", *graphics::Sampler::getLinearMirroredSampler());
        return defaultTexture;
    }

    static void printControls() {
        spdlog::info("ParallaxOcclusion Demo Controls:");
        spdlog::info("- WASD + EQ = Camera Movement");
        spdlog::info("- Mouse = Camera Yaw/Pitch");
        spdlog::info("- Use [UP], [DOWN], [LEFT], [RIGHT] to rotate around the x and z axes.");
        spdlog::info("- press [1] to toggle between fallback and wood texture");
        spdlog::info("- press [2] to toggle between fallback and wood phong");
        spdlog::info("- press [3] to toggle between fallback and wood normal");
        spdlog::info("- press [4] to toggle between fallback and wood heightmap");
        spdlog::info("- press [5] to exit this state");
    }

    void ParallaxOcclusionDemo::initializeHotkeys() {
        hotkey_toggleTexture_isDown = input::InputManager::isKeyPressed(input::Key::Num1);
        hotkey_togglePhong_isDown = input::InputManager::isKeyPressed(input::Key::Num2);
        hotkey_toggleNormal_isDown = input::InputManager::isKeyPressed(input::Key::Num3);
        hotkey_toggleHeightMap_isDown = input::InputManager::isKeyPressed(input::Key::Num4);
        hotkey_exit_isDown = input::InputManager::isKeyPressed(input::Key::Num5);
    }

    void ParallaxOcclusionDemo::initializeShaders() {
        graphics::Shader::Handle fragmentShader = graphics::ShaderManager::get("shader/demo.frag", graphics::ShaderType::FRAGMENT);
        graphics::Shader::Handle geometryShader = graphics::ShaderManager::get("shader/demo.geom", graphics::ShaderType::GEOMETRY);
        graphics::Shader::Handle vertexShader = graphics::ShaderManager::get("shader/demo.vert", graphics::ShaderType::VERTEX);

        program.attach(vertexShader);
        program.attach(geometryShader);
        program.attach(fragmentShader);
        program.link();
    }

    void ParallaxOcclusionDemo::loadShaders() {
        program.use();

        cameraControls.loadShaders(program.getID());
        glsl_ambient_light = glGetUniformLocation(program.getID(), "ambient_light");
    }

    void ParallaxOcclusionDemo::loadGeometry() {
        meshRenderer.clear();

        xyPlaneEntity = entity::EntityRegistry::getInstance().createEntity(
                components::Transform(glm::vec3(0.0f, 0.0f, 0.0f),
                                      glm::quat(glm::vec3(0.0f, 0.0f, 0.0f)),
                                      glm::vec3(1.0f, 1.0f, 1.0f)),
                components::Mesh(utils::MeshLoader::get("models/xyplane.obj"),
                                 getWoodTexture(), getWoodPhong(), getWoodNormal(), getWoodHeightMap())
        );
        meshRenderer.registerMesh(xyPlaneEntity);

        lightSource = entity::EntityRegistry::getInstance().createEntity(
                components::Light(graphics::LightData::directional(
                        glm::normalize(glm::vec3(-1.0f, -1.0f, 1.0f)),
                        glm::vec3(1.0f, 1.0f, 0.8f),
                        2.5f
                ))
        );
    }

    void ParallaxOcclusionDemo::bindLighting() {
        glUniform3fv(glsl_ambient_light, 1, glm::value_ptr(ambientLightData));
        graphics::LightManager::getInstance().bindLights(4);
    }

    ParallaxOcclusionDemo::ParallaxOcclusionDemo() :
            cameraControls(graphics::Camera(90.0f, 0.1f, 300.0f), glm::vec3(0.0f, 0.0f, -3.0f), 0.0f, 0.0f, 0.0f),
            ambientLightData({1.4f, 1.4f, 1.4f}),
            meshRenderer(graphics::MeshRenderer()) {

        initializeHotkeys();
        cameraControls.initializeCursorPosition();
        initializeShaders();
        loadShaders();
        loadGeometry();
        cameraControls.initializeScene();
        bindLighting();
        cameraControls.bindCamera();
        printControls();
    }

    void ParallaxOcclusionDemo::update(const long long &deltaMicroseconds) {
        if (!hotkey_toggleTexture_isDown && input::InputManager::isKeyPressed(input::Key::Num1)) {
            auto &registry = entity::EntityRegistry::getInstance();
            components::Mesh mesh = registry.getComponentData<components::Mesh>(xyPlaneEntity).value();
            mesh.setTextureData(mesh.getTextureData() == nullptr ? getWoodTexture() : nullptr);
            registry.addOrSetComponent(xyPlaneEntity, mesh);
        }
        if (!hotkey_togglePhong_isDown && input::InputManager::isKeyPressed(input::Key::Num2)) {
            auto &registry = entity::EntityRegistry::getInstance();
            components::Mesh mesh = registry.getComponentData<components::Mesh>(xyPlaneEntity).value();
            mesh.setPhongData(mesh.getPhongData() == nullptr ? getWoodPhong() : nullptr);
            registry.addOrSetComponent(xyPlaneEntity, mesh);
        }
        if (!hotkey_toggleNormal_isDown && input::InputManager::isKeyPressed(input::Key::Num3)) {
            auto &registry = entity::EntityRegistry::getInstance();
            components::Mesh mesh = registry.getComponentData<components::Mesh>(xyPlaneEntity).value();
            mesh.setNormalData(mesh.getNormalData() == nullptr ? getWoodNormal() : nullptr);
            registry.addOrSetComponent(xyPlaneEntity, mesh);
        }
        if (!hotkey_toggleHeightMap_isDown && input::InputManager::isKeyPressed(input::Key::Num4)) {
            auto &registry = entity::EntityRegistry::getInstance();
            components::Mesh mesh = registry.getComponentData<components::Mesh>(xyPlaneEntity).value();
            mesh.setHeightData(mesh.getHeightData() == nullptr ? getWoodHeightMap() : nullptr);
            registry.addOrSetComponent(xyPlaneEntity, mesh);
        }
        if (!hotkey_exit_isDown && input::InputManager::isKeyPressed(input::Key::Num5)) {
            onExit();
            return;
        }

        if (input::InputManager::isKeyPressed(input::Key::UP)) {
            auto &registry = entity::EntityRegistry::getInstance();
            components::Transform transform = registry.getComponentData<components::Transform>(xyPlaneEntity).value();
            transform.setRotation(transform.getRotation() * glm::quat(glm::vec3(glm::radians(0.5f), 0.0f, 0.0f)));
            registry.addOrSetComponent(xyPlaneEntity, transform);
        }
        if (input::InputManager::isKeyPressed(input::Key::DOWN)) {
            auto &registry = entity::EntityRegistry::getInstance();
            components::Transform transform = registry.getComponentData<components::Transform>(xyPlaneEntity).value();
            transform.setRotation(transform.getRotation() * glm::quat(glm::vec3(glm::radians(-0.5f), 0.0f, 0.0f)));
            registry.addOrSetComponent(xyPlaneEntity, transform);
        }
        if (input::InputManager::isKeyPressed(input::Key::RIGHT)) {
            auto &registry = entity::EntityRegistry::getInstance();
            components::Transform transform = registry.getComponentData<components::Transform>(xyPlaneEntity).value();
            transform.setRotation(transform.getRotation() * glm::quat(glm::vec3(0.0f, 0.0f, glm::radians(0.5f))));
            registry.addOrSetComponent(xyPlaneEntity, transform);
        }
        if (input::InputManager::isKeyPressed(input::Key::LEFT)) {
            auto &registry = entity::EntityRegistry::getInstance();
            components::Transform transform = registry.getComponentData<components::Transform>(xyPlaneEntity).value();
            transform.setRotation(transform.getRotation() * glm::quat(glm::vec3(0.0f, 0.0f, glm::radians(-0.5f))));
            registry.addOrSetComponent(xyPlaneEntity, transform);
        }

        initializeHotkeys();
        cameraControls.update(deltaMicroseconds);
    }

    void ParallaxOcclusionDemo::draw(const long long &deltaMicroseconds) {
        auto &registry = entity::EntityRegistry::getInstance();
        graphics::LightManager::LightSystem(registry).execute();

        meshRenderer.update();
        meshRenderer.present(program.getID());
    }

    void ParallaxOcclusionDemo::onResume() {
        initializeHotkeys();
        cameraControls.initializeCursorPosition();
        loadShaders();
        bindLighting();
        cameraControls.bindCamera();
        printControls();
    }

    void ParallaxOcclusionDemo::onPause() {
        spdlog::info("===== ParallaxOcclusion Demo State paused =====");
    }

    void ParallaxOcclusionDemo::onExit() {
        spdlog::info("exiting ParallaxOcclusion Demo state");

        entity::EntityRegistry::getInstance().eraseEntity(xyPlaneEntity);
        meshRenderer.clear();
        delete xyPlaneEntity;

        graphics::LightManager::getInstance().removeLight(entity::EntityRegistry::getInstance().getComponentData<components::Light>(lightSource).value());
        entity::EntityRegistry::getInstance().eraseEntity(lightSource);
        delete lightSource;

        _isFinished = true;
    }
}