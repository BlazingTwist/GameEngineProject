#include "collisionstate.h"

namespace gameState {

    struct CollisionState_TreeProcessor {
        CollisionState_TreeProcessor(const math::AABB<3, float> &bullet,
                                     std::vector<entity::EntityReference *> &_planetVec,
                                     graphics::MeshRenderer &_meshRenderer)
                : m_bullet(bullet), planetVec(_planetVec), meshRenderer(_meshRenderer) {}

        const math::AABB<3, float> &m_bullet;

        std::vector<entity::EntityReference *> &planetVec;
        graphics::MeshRenderer &meshRenderer;

        bool descend(const math::AABB<3, float> &aabb) {
            return aabb.intersect(m_bullet);
        }

        void process(const math::AABB<3, float> &aabb, const entity::EntityReference *planet) {
            if (aabb.intersect(m_bullet)) {
                meshRenderer.removeMesh(planet);
                const int planetID = planet->getReferenceID();
                for (unsigned int i = planetVec.size() - 1; i >= 0; i--) {
                    entity::EntityReference *storedPlanet = planetVec[i];
                    if (storedPlanet->getReferenceID() == planetID) {
                        entity::EntityRegistry::getInstance().eraseEntity(storedPlanet);
                        planetVec.erase(planetVec.begin() + i);
                        delete storedPlanet;
                        break;
                    }
                }
            }
        }
    };

    static constexpr auto defaultPlanetScale = glm::vec3(1.0f, 1.0f, 1.0f);
    static constexpr auto defaultBulletScale = glm::vec3(0.1f, 0.1f, 0.1f);
    static constexpr float defaultBulletVelocity = 10.0f;
    static constexpr float boxSize = 20.0f;

    static void printControls() {
        spdlog::info("Spring Demo Controls:");
        spdlog::info("- WASD + EQ = Camera Movement");
        spdlog::info("- Mouse = Camera Yaw/Pitch");
        spdlog::info("- LeftClick to spawn a projectile in view direction");
        spdlog::info("- press [1] to exit this state");
    }

    void CollisionState::initializeHotkeys() {
        hotkey_exit_isDown = input::InputManager::isKeyPressed(input::Key::Num1);
    }

    void CollisionState::initializeShaders() {
        graphics::Shader::Handle fragmentShader = graphics::ShaderManager::get("shader/demo.frag", graphics::ShaderType::FRAGMENT);
        graphics::Shader::Handle geometryShader = graphics::ShaderManager::get("shader/demo.geom", graphics::ShaderType::GEOMETRY);
        graphics::Shader::Handle vertexShader = graphics::ShaderManager::get("shader/demo.vert", graphics::ShaderType::VERTEX);

        program.attach(vertexShader);
        program.attach(geometryShader);
        program.attach(fragmentShader);
        program.link();
    }

    void CollisionState::loadShaders() {
        program.use();
        cameraControls.loadShaders(program.getID());
        glsl_ambient_light = glGetUniformLocation(program.getID(), "ambient_light");
    }

    void CollisionState::loadGeometry() {
        lightSource = entity::EntityRegistry::getInstance().createEntity(
                components::Light::directional(
                        glm::vec3(-1.0f, -1.0f, 0.5f), glm::vec3(1.0f, 1.0f, 0.8f), 0.75f
                )
        );
    }

    void CollisionState::createBullet() {
        glm::vec3 spawnPosition = cameraControls.camera.getPosition();
        glm::vec3 bulletDirection = cameraControls.camera.forwardVector();

        entity::EntityReference *bullet = entity::EntityRegistry::getInstance().createEntity(
                components::Transform(spawnPosition,
                                      glm::quat(glm::vec3(0.0f, 0.0f, 0.0f)),
                                      defaultBulletScale
                ),
                components::Mesh(utils::MeshLoader::get("models/sphere.obj"),
                                 graphics::Texture2DManager::get("textures/SunTexture.png", *graphics::Sampler::getLinearMirroredSampler())
                ),
                components::AABBCollider(),
                components::Velocity(bulletDirection * defaultBulletVelocity)
        );
        meshRenderer.registerMesh(bullet);
        bulletVec.push_back(bullet);

    }

    void CollisionState::bindLighting() {
        glUniform3fv(glsl_ambient_light, 1, glm::value_ptr(ambientLightData));
        graphics::LightManager::getInstance().bindLights(4);
    }

    CollisionState::CollisionState() :
            cameraControls(graphics::Camera(90.0f, 0.1f, 300.0f), glm::vec3(0.0f, 0.0f, -7.0f), 0.0f, 0.0f, 0.0f),
            ambientLightData({1.4f, 1.4f, 1.4f}),
            meshRenderer(graphics::MeshRenderer()),
            collisionTree(1.0f) {

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


    void CollisionState::createPlanets(const double &deltaSeconds) {
        if (planetVec.size() >= 20) {
            return;
        }

        nextPlanetSpawnSeconds -= deltaSeconds;
        if (nextPlanetSpawnSeconds <= 0) {
            nextPlanetSpawnSeconds = 1.0;

            std::random_device randomDevice;
            std::mt19937 gen(randomDevice());
            std::uniform_int_distribution<> positionDistribution(0, 9);
            std::uniform_int_distribution<> directionDistribution(0, 8);
            std::uniform_real_distribution<> angularVelocityDistribution(-1, 1);

            glm::vec3 position = glm::vec3(positionDistribution(gen), positionDistribution(gen), positionDistribution(gen));
            glm::vec3 direction = glm::vec3(directionDistribution(gen) - 4, directionDistribution(gen) - 4, directionDistribution(gen) - 4);
            glm::vec3 angularVelocity = glm::vec3(
                    static_cast<float>(angularVelocityDistribution(gen)),
                    static_cast<float>(angularVelocityDistribution(gen)),
                    static_cast<float>(angularVelocityDistribution(gen))
            );
            entity::EntityReference *planetEntity = entity::EntityRegistry::getInstance().createEntity(
                    components::Transform(position,
                                          glm::quat(glm::vec3(0.0f, 0.0f, 0.0f)),
                                          defaultPlanetScale
                    ),
                    components::Mesh(utils::MeshLoader::get("models/sphere.obj"),
                                     graphics::Texture2DManager::get("textures/planet1.png", *graphics::Sampler::getLinearMirroredSampler()),
                                     graphics::Texture2DManager::get("textures/Planet1_phong.png", *graphics::Sampler::getLinearMirroredSampler())
                    ),
                    components::Velocity(direction),
                    components::RotationalVelocity(angularVelocity),
                    components::AABBCollider()
            );
            meshRenderer.registerMesh(planetEntity);
            planetVec.push_back(planetEntity);
        }
    }

    void CollisionState::update(const long long &deltaMicroseconds) {
        const double deltaSeconds = (double) deltaMicroseconds / 1'000'000.0;
        const double deltaSecondsSquared = deltaSeconds * deltaSeconds;

        if (!hotkey_exit_isDown && input::InputManager::isKeyPressed(input::Key::Num1)) {
            onExit();
            return;
        }

        entity::EntityRegistry &registry = entity::EntityRegistry::getInstance();

        components::ApplyVelocitySystem(registry, deltaSeconds, deltaSecondsSquared).execute();
        components::ApplyRotationalVelocitySystem(registry, deltaSeconds, deltaSecondsSquared).execute();

        if (!planetVec.empty()) {
            registry.execute([deltaSeconds, &registry]
                                     (const entity::EntityReference *entity, components::Transform transform, components::Velocity velocity) {
                glm::vec3 pos = transform.getPosition();
                if (pos.x < -boxSize || pos.x > boxSize) velocity.velocity.x = -velocity.velocity.x;
                if (pos.y < -boxSize || pos.y > boxSize) velocity.velocity.y = -velocity.velocity.y;
                if (pos.z < -boxSize || pos.z > boxSize) velocity.velocity.z = -velocity.velocity.z;
                registry.addOrSetComponent(entity, velocity);
            });

            for (const entity::EntityReference *planetEntity: planetVec) {
                components::AABBCollider planetCollider = registry.getComponentData<components::AABBCollider>(planetEntity).value();
                components::Transform planetTransform = registry.getComponentData<components::Transform>(planetEntity).value();
                collisionTree.insert(planetCollider.getAABB(planetTransform), planetEntity);
            }

            for (const entity::EntityReference *bulletEntity: bulletVec) {
                components::AABBCollider bulletCollider = registry.getComponentData<components::AABBCollider>(bulletEntity).value();
                components::Transform bulletTransform = registry.getComponentData<components::Transform>(bulletEntity).value();
                CollisionState_TreeProcessor treeProc(bulletCollider.getAABB(bulletTransform), planetVec, meshRenderer);
                collisionTree.traverse(treeProc);
            }

            collisionTree.clear();
        }

        createPlanets(deltaSeconds);
        initializeHotkeys();
        cameraControls.update(deltaMicroseconds);

        if (bulletCoolDownSeconds > 0.0) {
            bulletCoolDownSeconds -= deltaSeconds;
        } else if (input::InputManager::isButtonPressed(input::MouseButton::LEFT)) {
            bulletCoolDownSeconds = 1.0;
            createBullet();
        }

        meshRenderer.update();
        graphics::LightManager::LightSystem(registry).execute();
    }

    void CollisionState::draw(const long long &deltaMicroseconds) {
        meshRenderer.present(program.getID());
    }

    void CollisionState::onResume() {
        printControls();
        initializeHotkeys();
        cameraControls.initializeCursorPosition();
        loadShaders();
        bindLighting();
        cameraControls.bindCamera();
    }

    void CollisionState::onPause() {
        spdlog::info("===== Collision state paused =====");
    }

    void CollisionState::onExit() {
        spdlog::info("exiting Collision State");

        meshRenderer.clear();

        for (entity::EntityReference *entity: bulletVec) {
            entity::EntityRegistry::getInstance().eraseEntity(entity);
            delete entity;
        }
        bulletVec.clear();

        for (entity::EntityReference *entity: planetVec) {
            entity::EntityRegistry::getInstance().eraseEntity(entity);
            delete entity;
        }
        planetVec.clear();

        graphics::LightManager::getInstance().removeLight(lightSource);
        entity::EntityRegistry::getInstance().eraseEntity(lightSource);
        delete lightSource;

        _isFinished = true;
    }
}