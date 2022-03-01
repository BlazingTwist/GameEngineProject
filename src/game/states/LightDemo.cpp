#include "LightDemo.h"

namespace gameState {

    static void printControls() {
        spdlog::info("Light Demo Controls:");
        spdlog::info("- WASD + EQ = Camera Movement");
        spdlog::info("- Mouse = Camera Yaw/Pitch");
        spdlog::info("- press [1] to spawn a spotlight at your position");
        spdlog::info("- press [2] to destroy the spotlight with least distance to your position");
        spdlog::info("- press [3] to exit this state");
    }

    void LightDemo::initializeHotkeys() {
        hotkey_spawnLight_isDown = input::InputManager::isKeyPressed(input::Key::Num1);
        hotkey_destroyLight_isDown = input::InputManager::isKeyPressed(input::Key::Num2);
        hotkey_exit_isDown = input::InputManager::isKeyPressed(input::Key::Num3);
    }

    void LightDemo::initializeShaders() {
        graphics::Shader::Handle fragmentShader = graphics::ShaderManager::get("shader/demo.frag", graphics::ShaderType::FRAGMENT);
        graphics::Shader::Handle geometryShader = graphics::ShaderManager::get("shader/demo.geom", graphics::ShaderType::GEOMETRY);
        graphics::Shader::Handle vertexShader = graphics::ShaderManager::get("shader/demo.vert", graphics::ShaderType::VERTEX);

        program.attach(vertexShader);
        program.attach(geometryShader);
        program.attach(fragmentShader);
        program.link();
    }

    void LightDemo::loadShaders() {
        program.use();

        cameraControls.loadShaders(program.getID());
        glsl_ambient_light = glGetUniformLocation(program.getID(), "ambient_light");
    }

    void LightDemo::loadGeometry() {
        meshRenderer.clear();

        entity::EntityRegistry &registry = entity::EntityRegistry::getInstance();

        xyPlaneEntity = registry.createEntity(
                components::Transform(glm::vec3(0.0f, 0.0f, 0.0f),
                                      glm::quat(glm::vec3(0.0f, 0.0f, 0.0f)),
                                      glm::vec3(1.0f, 1.0f, 1.0f)),
                components::Mesh(utils::MeshLoader::get("models/xyplane.obj"),
                                 graphics::Texture2DManager::get("textures/woodlogwall/texture.png", *graphics::Sampler::getLinearMirroredSampler()),
                                 graphics::Texture2DManager::get("textures/woodlogwall/phong.png", *graphics::Sampler::getLinearMirroredSampler()),
                                 graphics::Texture2DManager::get("textures/woodlogwall/normal.png", *graphics::Sampler::getLinearMirroredSampler()),
                                 graphics::Texture2DManager::get("textures/woodlogwall/heightmap.png", *graphics::Sampler::getLinearMirroredSampler()))
        );
        meshRenderer.registerMesh(xyPlaneEntity);

        shipEntity = registry.createEntity(
                components::Mesh(utils::MeshLoader::get("models/starSparrow/mesh01.obj"),
                                 graphics::Texture2DManager::get("models/starSparrow/texture/StarSparrow_Green.png",
                                                                 *graphics::Sampler::getLinearMirroredSampler()),
                                 graphics::Texture2DManager::get("models/starSparrow/phong.png", *graphics::Sampler::getLinearMirroredSampler()),
                                 graphics::Texture2DManager::get("models/starSparrow/normal.png", *graphics::Sampler::getLinearMirroredSampler())
                ),
                components::Transform(glm::vec3(10.0f, 0.0f, 0.0f),
                                      glm::quat(glm::vec3(0.0f, 0.0f, 0.0f)),
                                      glm::vec3(1.0f, 1.0f, 1.0f)
                )
        );
        meshRenderer.registerMesh(shipEntity);

        const utils::MeshData::Handle sphereData = utils::MeshLoader::get("models/sphere.obj");
        sphereInvertedMeshData = new utils::MeshData;
        sphereInvertedMeshData->positions = sphereData->positions;
        sphereInvertedMeshData->faces = sphereData->faces;
        sphereInvertedMeshData->textureCoordinates = sphereData->textureCoordinates;
        sphereInvertedMeshData->normals = {};
        for (const auto &normal: sphereData->normals) {
            sphereInvertedMeshData->normals.push_back(normal * -1.0f);
        }
    }

    void LightDemo::bindLighting() {
        glUniform3fv(glsl_ambient_light, 1, glm::value_ptr(ambientLightData));
        graphics::LightManager::getInstance().bindLights(4);
    }

    LightDemo::LightDemo() :
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

    static glm::vec3 getRandomColor() {
        std::random_device randomDevice;
        std::mt19937 gen(randomDevice());
        std::uniform_int_distribution<> colorDistribution(0, 255);

        float red = static_cast<float>(colorDistribution(gen)) / 255.0f;
        float green = static_cast<float>(colorDistribution(gen)) / 255.0f;
        float blue = static_cast<float>(colorDistribution(gen)) / 255.0f;

        return {red, green, blue};
    }

    void LightDemo::update(const long long &deltaMicroseconds) {
        entity::EntityRegistry &registry = entity::EntityRegistry::getInstance();

        if (!hotkey_spawnLight_isDown && input::InputManager::isKeyPressed(input::Key::Num1)) {
            const glm::vec3 &position = cameraControls.camera.getPosition();
            const glm::vec3 &camForward = cameraControls.camera.forwardVector();
            const glm::quat camRotation = cameraControls.camera.getRotationAsQuaternion();
            entity::EntityReference *newLightEntity = registry.createEntity(
                    components::Light::spot(position, camForward, 50.0f, 22.5f, getRandomColor(), 0.75f),
                    components::Mesh(sphereInvertedMeshData,
                                     graphics::Texture2DManager::get("textures/SunTexture.png", *graphics::Sampler::getLinearMirroredSampler()),
                                     graphics::Texture2DManager::get("textures/Sun_phong.png", *graphics::Sampler::getLinearMirroredSampler())
                    ),
                    components::Transform(position, camRotation, glm::vec3(0.1f, 0.1f, 0.1f))
            );
            activeLights.push_back(newLightEntity);
            meshRenderer.registerMesh(newLightEntity);
        }

        if (!hotkey_destroyLight_isDown && input::InputManager::isKeyPressed(input::Key::Num2)) {
            if (activeLights.empty()) {
                spdlog::info("No lights left to be destroyed! Try spawning one first.");
            } else {
                const glm::vec3 &camPosition = cameraControls.camera.getPosition();
                int nearestLightEntity = -1;
                float minDistance = 0.0f;
                const int lightCount = static_cast<int>(activeLights.size());
                for (int i = 0; i < lightCount; ++i) {
                    components::Light lightComp = registry.getComponentData<components::Light>(activeLights[i]).value();
                    float distance = glm::length(camPosition - lightComp.getPosition());
                    if (distance < minDistance || nearestLightEntity < 0) {
                        nearestLightEntity = i;
                        minDistance = distance;
                    }
                }

                entity::EntityReference *lightEntity = activeLights[nearestLightEntity];
                graphics::LightManager::getInstance().removeLight(lightEntity);
                meshRenderer.removeMesh(lightEntity);
                registry.eraseEntity(lightEntity);
                activeLights.erase(activeLights.begin() + nearestLightEntity);
                delete lightEntity;
            }
        }

        if (!hotkey_exit_isDown && input::InputManager::isKeyPressed(input::Key::Num3)) {
            onExit();
            return;
        }

        initializeHotkeys();
        cameraControls.update(deltaMicroseconds);
        meshRenderer.update();
        graphics::LightManager::LightSystem(registry).execute();
    }

    void LightDemo::draw(const long long &deltaMicroseconds) {
        meshRenderer.present(program.getID());
    }

    void LightDemo::onResume() {
        initializeHotkeys();
        cameraControls.initializeCursorPosition();
        loadShaders();
        bindLighting();
        cameraControls.bindCamera();
        printControls();
    }

    void LightDemo::onPause() {
        spdlog::info("===== Light Demo State paused =====");
    }

    void LightDemo::onExit() {
        spdlog::info("exiting Light Demo state");

        meshRenderer.clear();

        entity::EntityRegistry::getInstance().eraseEntity(xyPlaneEntity);
        delete xyPlaneEntity;

        entity::EntityRegistry::getInstance().eraseEntity(shipEntity);
        delete shipEntity;

        for (entity::EntityReference *entity: activeLights) {
            graphics::LightManager::getInstance().removeLight(entity::EntityRegistry::getInstance().getComponentData<components::Light>(entity).value());
            entity::EntityRegistry::getInstance().eraseEntity(entity);
            delete entity;
        }
        activeLights.clear();

        _isFinished = true;
    }
}