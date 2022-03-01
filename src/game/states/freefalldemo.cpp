#include "freefalldemo.h"

namespace gameState {
    static constexpr auto defaultPlanetVelocity = glm::vec3(0.0f, 0.0f, 0.0f);

    static void printControls() {
        spdlog::info("Free Fall Demo Controls:");
        spdlog::info("- press [1] to exit this state");
    }

    void FreeFallDemoState::initializeHotkeys() {
        hotkey_exit_isDown = input::InputManager::isKeyPressed(input::Key::Num1);
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

    void FreeFallDemoState::bindLighting() {
        glUniform3fv(glsl_ambient_light, 1, glm::value_ptr(ambientLightData));
        graphics::LightManager::getInstance().bindLights(4);
    }


    FreeFallDemoState::FreeFallDemoState() :
            cameraControls(graphics::Camera(90.0f, 0.1f, 300.0f), glm::vec3(0.0f, 0.0f, -7.0f), 0.0f, 0.0f, 0.0f),
            ambientLightData({1.4f, 1.4f, 1.4f}),
            meshRenderer(graphics::MeshRenderer()) {

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

    void FreeFallDemoState::createObjects(const double &deltaSeconds) {
        nextPlanetSpawnSeconds -= deltaSeconds;
        if (nextPlanetSpawnSeconds <= 0) {
            nextPlanetSpawnSeconds = 0.25;


            std::random_device randomDevice;
            std::mt19937 gen(randomDevice());
            std::uniform_real_distribution<> positionDistribution(-4, 4);
            std::uniform_real_distribution<> scaleDistribution(0.1, 1.5);

            glm::vec3 position = glm::vec3(positionDistribution(gen), 15, positionDistribution(gen));
            glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f) * static_cast<float>(scaleDistribution(gen));

            entity::EntityReference *planetEntity = entity::EntityRegistry::getInstance().createEntity(
                    components::Transform(position,
                                          glm::quat(glm::vec3(0.0f, 0.0f, 0.0f)),
                                          scale),
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
        if (!hotkey_exit_isDown && input::InputManager::isKeyPressed(input::Key::Num1)) {
            onExit();
            return;
        }
        initializeHotkeys();

        double deltaSeconds = (double) deltaMicroseconds / 1'000'000.0;
        double deltaSecondsSquared = deltaSeconds * deltaSeconds;

        entity::EntityRegistry &registry = entity::EntityRegistry::getInstance();

        std::vector<const entity::EntityReference *> outOfBoundsEntities = {};
        registry.execute([deltaSeconds, deltaSecondsSquared, &outOfBoundsEntities]
                                 (const entity::EntityReference *entity, components::Transform transform, components::PhysicsObject phys) {
            static constexpr float gravityAcceleration = 1.0f;
            double velocityGain = gravityAcceleration * deltaSeconds;
            double accelerationDistance = gravityAcceleration * deltaSecondsSquared;

            glm::vec3 positionDelta =
                    (glm::vec3(0, -1, 0) * static_cast<float>(accelerationDistance)) + (phys._velocity * static_cast<float>(deltaSeconds));

            phys._velocity += (glm::vec3(0, -1, 0) * static_cast<float>(velocityGain));
            transform.setPosition(transform.getPosition() + positionDelta);

            // remove entities when fallen below y=-20;
            if (transform.getPosition().y <= -20) {
                outOfBoundsEntities.push_back(entity);
            }

            entity::EntityRegistry::getInstance().addOrSetComponent(entity, transform);
            entity::EntityRegistry::getInstance().addOrSetComponent(entity, phys);
        });

        for (const entity::EntityReference *outOfBoundsEntity: outOfBoundsEntities) {
            for (int i = static_cast<int>(planetVec.size() - 1); i >= 0; i--) {
                entity::EntityReference *entity = planetVec[i];
                if (entity->getReferenceID() == outOfBoundsEntity->getReferenceID()) {
                    meshRenderer.removeMesh(entity);
                    registry.eraseEntity(entity);
                    planetVec.erase(planetVec.begin() + i);
                    delete entity;
                }
            }
        }

        createObjects(deltaSeconds);
        
        meshRenderer.update();
        graphics::LightManager::LightSystem(registry).execute();
    }


    void FreeFallDemoState::draw(const long long &deltaMicroseconds) {
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
        spdlog::info("exiting free fall demo state");

        meshRenderer.clear();
        
        entity::EntityRegistry &registry = entity::EntityRegistry::getInstance();
        for (entity::EntityReference *entity : planetVec){
            registry.eraseEntity(entity);
            delete entity;
        }
        planetVec.clear();

        graphics::LightManager::getInstance().removeLight(lightSource);
        entity::EntityRegistry::getInstance().eraseEntity(lightSource);
        delete lightSource;

        _isFinished = true;
    }
}