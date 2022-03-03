#include "SpaceSim.h"

namespace gameState {

    using texManager = graphics::Texture2DManager;
    static constexpr auto linearMirrorSampler = graphics::Sampler::getLinearMirroredSampler;
    static constexpr auto cameraRotationButton = input::MouseButton::LEFT;
    static constexpr auto mouseSensitivity = glm::vec2(2.0f, 2.0f);
    static constexpr auto cameraForwardKey = input::Key::I;
    static constexpr auto cameraBackwardKey = input::Key::K;
    static constexpr auto cameraLeftKey = input::Key::J;
    static constexpr auto cameraRightKey = input::Key::L;
    static constexpr auto cameraUpKey = input::Key::O;
    static constexpr auto cameraDownKey = input::Key::U;
    static constexpr auto cameraMoveSensitivity = glm::vec3(1.0f, 1.0f, 1.0f);

    static constexpr auto shipPitchSensitivity = 0.25f;
    static constexpr auto shipRollSensitivity = 0.25f;
    static constexpr auto shipYawSensitivity = 0.25f;
    static constexpr auto shipStrafeSensitivity = 0.25f;
    static constexpr auto shipThrustFactor = 1.0f;
    static constexpr auto shipRotationDampFactor = 0.9f;
    static constexpr auto shipVelocityDampFactor = 2.0f;

    static constexpr auto projectileVelocity = 100.0f;
    static constexpr auto projectileLifetime = 1.25;
    static constexpr auto projectileLightIntensity = 3.0f;

    static void printControls() {
        spdlog::info("SpaceSim Controls:");
        spdlog::info("- W/S = Pitch Down/Up");
        spdlog::info("- A/D = Roll Left/Right");
        spdlog::info("- Q/E = Yaw Left/Right");
        spdlog::info("- UP/DOWN = Strafe up/down");
        spdlog::info("- LEFT/RIGHT = Strafe left/right");
        spdlog::info("- R/F = Increase/Decrease throttle along Thrust Axis");
        spdlog::info("- X = Enable Rotation Dampeners");
        spdlog::info("- C = Enable Velocity Dampeners");
        spdlog::info("- Space-Bar = Fire Laser");
        spdlog::info("Camera Controls");
        spdlog::info("- press [1] for 3rd person follow camera");
        spdlog::info("- press [2] for 3rd person reverse camera");
        spdlog::info("- press [3] for landing camera");
        spdlog::info("- hold left-click and drag to adjust camera view direction");
        spdlog::info("- use I/J/K/L and U/O to adjust camera position");
        spdlog::info("- press [M] to reset active camera position and rotation");
        spdlog::info("State Controls");
        spdlog::info("- press [P] to exit this state");
    }

    void SpaceSim::initializeHotkeys() {
        hotkey_camera1_isDown = input::InputManager::isKeyPressed(input::Key::Num1);
        hotkey_camera2_isDown = input::InputManager::isKeyPressed(input::Key::Num2);
        hotkey_camera3_isDown = input::InputManager::isKeyPressed(input::Key::Num3);
        hotkey_cameraReset_isDown = input::InputManager::isKeyPressed(input::Key::M);
        hotkey_exit_isDown = input::InputManager::isKeyPressed(input::Key::P);
        throttleKeyIsDown = input::InputManager::isKeyPressed(input::Key::R)
                            || input::InputManager::isKeyPressed(input::Key::F);
    }

    void SpaceSim::checkHotkeys(const long long int &deltaMicroseconds) {
        if (!hotkey_camera1_isDown && input::InputManager::isKeyPressed(input::Key::Num1)) {
            activeFollowCamera = &camera1;
        }
        if (!hotkey_camera2_isDown && input::InputManager::isKeyPressed(input::Key::Num2)) {
            activeFollowCamera = &camera2;
        }
        if (!hotkey_camera3_isDown && input::InputManager::isKeyPressed(input::Key::Num3)) {
            activeFollowCamera = &camera3;
        }
        if (!hotkey_cameraReset_isDown && input::InputManager::isKeyPressed(input::Key::M)) {
            activeFollowCamera->resetCameraOffsets();
        }
        if (!hotkey_exit_isDown && input::InputManager::isKeyPressed(input::Key::P)) {
            onExit();
        }
    }

    void SpaceSim::initializeShaders() {
        graphics::Shader::Handle fragmentShader = graphics::ShaderManager::get("shader/demo.frag", graphics::ShaderType::FRAGMENT);
        graphics::Shader::Handle geometryShader = graphics::ShaderManager::get("shader/demo.geom", graphics::ShaderType::GEOMETRY);
        graphics::Shader::Handle vertexShader = graphics::ShaderManager::get("shader/demo.vert", graphics::ShaderType::VERTEX);

        program.attach(vertexShader);
        program.attach(geometryShader);
        program.attach(fragmentShader);
        program.link();
    }

    void SpaceSim::loadShaders() {
        program.use();

        camera1.loadShaders(program.getID());
        camera2.loadShaders(program.getID());
        camera3.loadShaders(program.getID());
        glsl_ambient_light = glGetUniformLocation(program.getID(), "ambient_light");
    }

    void SpaceSim::prepareEntities() {
        meshRenderer.clear();
        entity::EntityRegistry &registry = entity::EntityRegistry::getInstance();

        playerShipEntity = registry.createEntity(
                components::Transform(glm::vec3(0.0f, 240.0f, 0.0f),
                                      glm::quat(glm::vec3(0.0f, 0.0f, 0.0f)),
                                      glm::vec3(1.0f, 1.0f, 1.0f)
                ),
                components::Mesh(utils::MeshLoader::get("models/starSparrow/mesh01.obj"),
                                 texManager::get("models/starSparrow/texture/StarSparrow_Green.png", *linearMirrorSampler()),
                                 texManager::get("models/starSparrow/phong.png", *linearMirrorSampler()),
                                 texManager::get("models/starSparrow/normal.png", *linearMirrorSampler())
                ),
                components::RotationalVelocity(glm::vec3(0.0f, 0.0f, 0.0f)),
                components::Velocity(glm::vec3(0.0f, 0.0f, 5.5f)),
                components::OrbitalObject(10.0)
        );
        meshRenderer.registerMesh(playerShipEntity);
        camera1.trackEntity(playerShipEntity);
        camera2.trackEntity(playerShipEntity);
        camera3.trackEntity(playerShipEntity);

        const utils::MeshData::Handle sphereData = utils::MeshLoader::get("models/midPolyUVSphere.obj");
        sphereInvertedMeshData = new utils::MeshData;
        sphereInvertedMeshData->positions = sphereData->positions;
        sphereInvertedMeshData->faces = sphereData->faces;
        sphereInvertedMeshData->textureCoordinates = sphereData->textureCoordinates;
        sphereInvertedMeshData->normals = {};
        for (const auto &normal: sphereData->normals) {
            sphereInvertedMeshData->normals.push_back(normal * -1.0f);
        }

        entity::EntityReference *planetEntity;
        planetEntity = registry.createEntity( // Central Sun
                components::Mesh(sphereInvertedMeshData),
                components::Transform(glm::vec3(0.0f, 0.0f, 0.0f),
                                      glm::quat(glm::vec3(0.0f, 0.0f, 0.0f)),
                                      glm::vec3(200.0f, 200.0f, 200.0f)
                ),
                components::Velocity(glm::vec3(0.0f, 0.0f, 0.0f)),
                components::OrbitalObject(100'000'000.0),
                components::RotationalVelocity(glm::vec3(0.0f, 0.0f, 0.0f)),
                components::Light::point(glm::vec3(0.0f, 0.0f, 0.0f), 2000.0f, glm::vec3(1.0f, 0.0f, 0.0f), 1.0f)
        );
        meshRenderer.registerMesh(planetEntity);
        solarSystemEntities.push_back(planetEntity);

        planetEntity = registry.createEntity( // Inner Orbit 1
                components::Mesh(sphereInvertedMeshData),
                components::Transform(glm::vec3(0.0f, -400.0f, 0.0f),
                                      glm::quat(glm::vec3(0.0f, 0.0f, 0.0f)),
                                      glm::vec3(30.0f, 30.0f, 30.0f)
                ),
                components::Velocity(glm::vec3(4.7f, 0.0f, 1.0f)),
                components::OrbitalObject(20'000.0),
                components::RotationalVelocity(glm::vec3(0.0f, 0.0f, 0.0f)),
                components::Light::point(glm::vec3(0.0f, 0.0f, 0.0f), 300.0f, glm::vec3(0.0f, 1.0f, 0.0f), 1.0f)
        );
        meshRenderer.registerMesh(planetEntity);
        solarSystemEntities.push_back(planetEntity);

        planetEntity = registry.createEntity( // Inner Orbit 2
                components::Mesh(sphereInvertedMeshData),
                components::Transform(glm::vec3(0.0f, -500.0f, 0.0f),
                                      glm::quat(glm::vec3(0.0f, 0.0f, 0.0f)),
                                      glm::vec3(40.0f, 40.0f, 40.0f)
                ),
                components::Velocity(glm::vec3(3.9f, 0.0f, 1.0f)),
                components::OrbitalObject(20'000.0),
                components::RotationalVelocity(glm::vec3(0.0f, 0.0f, 0.0f)),
                components::Light::point(glm::vec3(0.0f, 0.0f, 0.0f), 400.0f, glm::vec3(0.0f, 1.0f, 1.0f), 1.0f)
        );
        meshRenderer.registerMesh(planetEntity);
        solarSystemEntities.push_back(planetEntity);

        planetEntity = registry.createEntity( // Medium Orbit
                components::Mesh(sphereInvertedMeshData),
                components::Transform(glm::vec3(0.0f, 0.0f, -1200.0f),
                                      glm::quat(glm::vec3(0.0f, 0.0f, 0.0f)),
                                      glm::vec3(75.0f, 75.0f, 75.0f)
                ),
                components::Velocity(glm::vec3(1.0f, 2.5f, 0.0f)),
                components::OrbitalObject(100'000.0),
                components::RotationalVelocity(glm::vec3(0.0f, 0.0f, 0.0f)),
                components::Light::point(glm::vec3(0.0f, 0.0f, 0.0f), 750.0f, glm::vec3(0.75f, 0.5f, 0.0f), 1.0f)
        );
        meshRenderer.registerMesh(planetEntity);
        solarSystemEntities.push_back(planetEntity);

        planetEntity = registry.createEntity( // Medium Orbit - Moon 1
                components::Mesh(sphereInvertedMeshData),
                components::Transform(glm::vec3(0.0f, 0.0f, -1400.0f),
                                      glm::quat(glm::vec3(0.0f, 0.0f, 0.0f)),
                                      glm::vec3(15.0f, 15.0f, 15.0f)
                ),
                components::Velocity(glm::vec3(1.0f, 2.1f, 0.0f)),
                components::OrbitalObject(1'000.0),
                components::RotationalVelocity(glm::vec3(0.0f, 0.0f, 0.0f)),
                components::Light::point(glm::vec3(0.0f, 0.0f, 0.0f), 150.0f, glm::vec3(1.0f, 0.75f, 0.0f), 1.0f)
        );
        meshRenderer.registerMesh(planetEntity);
        solarSystemEntities.push_back(planetEntity);

        planetEntity = registry.createEntity( // Outer Orbit
                components::Mesh(sphereInvertedMeshData),
                components::Transform(glm::vec3(-3400.0f, 0.0f, 500.0f),
                                      glm::quat(glm::vec3(0.0f, 0.0f, 0.0f)),
                                      glm::vec3(100.0f, 100.0f, 100.0f)
                ),
                components::Velocity(glm::vec3(0.0f, -0.5f, 1.5f)),
                components::OrbitalObject(500'000.0),
                components::RotationalVelocity(glm::vec3(0.0f, 0.0f, 0.0f)),
                components::Light::point(glm::vec3(0.0f, 0.0f, 0.0f), 1000.0f, glm::vec3(0.75f, 0.0f, 0.75f), 1.0f)
        );
        meshRenderer.registerMesh(planetEntity);
        solarSystemEntities.push_back(planetEntity);

        planetEntity = registry.createEntity( // Outer Orbit - Moon 1
                components::Mesh(sphereInvertedMeshData),
                components::Transform(glm::vec3(-3550.0f, 0.0f, 500.0f),
                                      glm::quat(glm::vec3(0.0f, 0.0f, 0.0f)),
                                      glm::vec3(20.0f, 20.0f, 20.0f)
                ),
                components::Velocity(glm::vec3(0.0f, -0.5f, 1.03f)),
                components::OrbitalObject(3'000.0),
                components::RotationalVelocity(glm::vec3(0.0f, 0.0f, 0.0f)),
                components::Light::point(glm::vec3(0.0f, 0.0f, 0.0f), 200.0f, glm::vec3(1.0f, 0.0f, 1.0f), 1.0f)
        );
        meshRenderer.registerMesh(planetEntity);
        solarSystemEntities.push_back(planetEntity);

        planetEntity = registry.createEntity( // Outer Orbit - Moon 2
                components::Mesh(sphereInvertedMeshData),
                components::Transform(glm::vec3(-3720.0f, 0.0f, 500.0f),
                                      glm::quat(glm::vec3(0.0f, 0.0f, 0.0f)),
                                      glm::vec3(30.0f, 30.0f, 30.0f)
                ),
                components::Velocity(glm::vec3(0.0f, -0.5f, 1.15f)),
                components::OrbitalObject(10'000.0),
                components::RotationalVelocity(glm::vec3(0.0f, 0.0f, 0.0f)),
                components::Light::point(glm::vec3(0.0f, 0.0f, 0.0f), 300.0f, glm::vec3(0.8f, 0.4f, 0.8f), 1.0f)
        );
        meshRenderer.registerMesh(planetEntity);
        solarSystemEntities.push_back(planetEntity);

        skyboxEntity = registry.createEntity( // very hacky skybox (sky-sphere)
                components::Mesh(sphereInvertedMeshData,
                                 graphics::Texture2DManager::get("textures/space_skybox/tex_argb.png", *graphics::Sampler::getLinearMirroredSampler()),
                                 graphics::Texture2DManager::get("textures/space_skybox/phong.png", *graphics::Sampler::getLinearMirroredSampler())
                ),
                components::Transform(glm::vec3(0.0f, 0.0f, 0.0f),
                                      glm::quat(glm::vec3(0.0f, 0.0f, 0.0f)),
                                      glm::vec3(8000.0f, 8000.0f, 8000.0f)
                )
        );
        meshRenderer.registerMesh(skyboxEntity);
    }

    void SpaceSim::bindLighting() {
        glUniform3fv(glsl_ambient_light, 1, glm::value_ptr(ambientLightData));
        graphics::LightManager::getInstance().bindLights(4);
    }

    SpaceSim::SpaceSim()
            : cameraInstance(90.0f, 0.1f, 20000.0f),
              camera1(cameraInstance,
                      glm::vec3(0.0f, 3.0f, -10.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                      cameraRotationButton, mouseSensitivity,
                      cameraForwardKey, cameraBackwardKey, cameraLeftKey, cameraRightKey, cameraUpKey, cameraDownKey, cameraMoveSensitivity
              ),
              camera2(cameraInstance,
                      glm::vec3(0.0f, 2.0f, 10.0f), glm::vec3(0.0f, glm::radians(180.0f), 0.0f),
                      cameraRotationButton, mouseSensitivity,
                      cameraForwardKey, cameraBackwardKey, cameraLeftKey, cameraRightKey, cameraUpKey, cameraDownKey, cameraMoveSensitivity
              ),
              camera3(cameraInstance,
                      glm::vec3(-5.0f, 3.0f, 3.0f), glm::vec3(glm::radians(60.0f), glm::radians(135.0f), 0.0f),
                      cameraRotationButton, mouseSensitivity,
                      cameraForwardKey, cameraBackwardKey, cameraLeftKey, cameraRightKey, cameraUpKey, cameraDownKey, cameraMoveSensitivity
              ),
              activeFollowCamera(&camera1),
              ambientLightData({1.4f, 1.4f, 1.4f}),
              meshRenderer(graphics::MeshRenderer()) {

        initializeHotkeys();
        initializeShaders();
        loadShaders();
        prepareEntities();
        bindLighting();
        activeFollowCamera->bindCamera();

        printControls();
    }

    void SpaceSim::handleFlightControls(const double deltaSeconds, const double deltaSecondsSquared) {
        entity::EntityRegistry &registry = entity::EntityRegistry::getInstance();
        components::Transform shipTransform = registry.getComponentData<components::Transform>(playerShipEntity).value();
        components::Velocity shipVelocity = registry.getComponentData<components::Velocity>(playerShipEntity).value();
        components::RotationalVelocity shipRotationVelocity = registry.getComponentData<components::RotationalVelocity>(playerShipEntity).value();

        if (input::InputManager::isKeyPressed(input::Key::W)) {
            shipRotationVelocity.eulerAngleVelocity += (glm::vec3(static_cast<float>(deltaSeconds) * shipPitchSensitivity, 0.0f, 0.0f));
        }
        if (input::InputManager::isKeyPressed(input::Key::S)) {
            shipRotationVelocity.eulerAngleVelocity -= (glm::vec3(static_cast<float>(deltaSeconds) * shipPitchSensitivity, 0.0f, 0.0f));
        }
        if (input::InputManager::isKeyPressed(input::Key::A)) {
            shipRotationVelocity.eulerAngleVelocity += (glm::vec3(0.0f, 0.0f, static_cast<float>(deltaSeconds) * shipRollSensitivity));
        }
        if (input::InputManager::isKeyPressed(input::Key::D)) {
            shipRotationVelocity.eulerAngleVelocity -= (glm::vec3(0.0f, 0.0f, static_cast<float>(deltaSeconds) * shipRollSensitivity));
        }
        if (input::InputManager::isKeyPressed(input::Key::Q)) {
            shipRotationVelocity.eulerAngleVelocity -= (glm::vec3(0.0f, static_cast<float>(deltaSeconds) * shipYawSensitivity, 0.0f));
        }
        if (input::InputManager::isKeyPressed(input::Key::E)) {
            shipRotationVelocity.eulerAngleVelocity += (glm::vec3(0.0f, static_cast<float>(deltaSeconds) * shipYawSensitivity, 0.0f));
        }

        if (input::InputManager::isKeyPressed(input::Key::UP)) {
            shipVelocity.velocity += (shipTransform.getRotation() * glm::vec3(0.0f, static_cast<float>(deltaSeconds) * shipStrafeSensitivity, 0.0f));
        }
        if (input::InputManager::isKeyPressed(input::Key::DOWN)) {
            shipVelocity.velocity -= (shipTransform.getRotation() * glm::vec3(0.0f, static_cast<float>(deltaSeconds) * shipStrafeSensitivity, 0.0f));
        }
        if (input::InputManager::isKeyPressed(input::Key::LEFT)) {
            shipVelocity.velocity -= (shipTransform.getRotation() * glm::vec3(static_cast<float>(deltaSeconds) * shipStrafeSensitivity, 0.0f, 0.0f));
        }
        if (input::InputManager::isKeyPressed(input::Key::RIGHT)) {
            shipVelocity.velocity += (shipTransform.getRotation() * glm::vec3(static_cast<float>(deltaSeconds) * shipStrafeSensitivity, 0.0f, 0.0f));
        }

        if (!throttleKeyIsDown && input::InputManager::isKeyPressed(input::Key::R)) {
            currentPlayerThrottle++;
            if (currentPlayerThrottle > 5) {
                spdlog::info("throttle maxed. (5)");
                currentPlayerThrottle = 5;
            } else {
                spdlog::info("throttle set to {}", currentPlayerThrottle);
            }
        }
        if (!throttleKeyIsDown && input::InputManager::isKeyPressed(input::Key::F)) {
            currentPlayerThrottle--;
            if (currentPlayerThrottle < -2) {
                spdlog::info("throttle maxed. (-2)");
                currentPlayerThrottle = -2;
            } else {
                spdlog::info("throttle set to {}", currentPlayerThrottle);
            }
        }
        if (currentPlayerThrottle != 0) {
            shipVelocity.velocity += (shipTransform.getRotation() *
                                      glm::vec3(0.0f, 0.0f, static_cast<float>(deltaSeconds) * shipThrustFactor * static_cast<float>(currentPlayerThrottle)));
        }

        if (input::InputManager::isKeyPressed(input::Key::X)) {
            shipRotationVelocity.eulerAngleVelocity *= shipRotationDampFactor;
        }
        if (input::InputManager::isKeyPressed(input::Key::C)) {
            glm::vec3 counterThrust = glm::normalize(shipVelocity.velocity) * (static_cast<float>(-deltaSeconds) * shipVelocityDampFactor);
            shipVelocity.velocity += counterThrust;
        }

        registry.addOrSetComponent(playerShipEntity, shipVelocity);
        registry.addOrSetComponent(playerShipEntity, shipRotationVelocity);

        if (remainingCannonCooldown > 0.0) {
            remainingCannonCooldown -= deltaSeconds;
        } else if (input::InputManager::isKeyPressed(input::Key::SPACE)) {
            remainingCannonCooldown = 0.1;
            glm::vec3 cannonOffset_left = cannonOffsets[currentCannonIndex];
            glm::vec3 cannonOffset_right = cannonOffset_left * glm::vec3(-1.0f, 0.0f, 0.0f);
            glm::vec3 projectileVelocityVec = shipTransform.getRotation() * glm::vec3(0.0f, 0.0f, projectileVelocity);
            spawnProjectile(shipTransform, cannonOffset_left, projectileVelocityVec);
            spawnProjectile(shipTransform, cannonOffset_right, projectileVelocityVec);
            currentCannonIndex = (currentCannonIndex + 1) % 4;
        }
    }

    void SpaceSim::spawnProjectile(const components::Transform &shipTransform, const glm::vec3 &spawnOffset, const glm::vec3 &velocity) {
        activeProjectiles.emplace_back(
                entity::EntityRegistry::getInstance().createEntity(
                        components::Mesh(sphereInvertedMeshData),
                        components::Transform(shipTransform.getPosition() + (shipTransform.getRotation() * spawnOffset),
                                              shipTransform.getRotation(),
                                              glm::vec3(0.1f, 0.1f, 1.0f)
                        ),
                        components::Velocity(velocity),
                        components::RotationalVelocity(glm::vec3(0.0f, 0.0f, glm::radians(45.0f))),
                        components::Light::point(glm::vec3(0.0f, 0.0f, 0.0f), 50.0f, glm::vec3(1.0f, 1.0f, 0.0f), 3.0f),
                        components::ScaleVelocity(glm::vec3(0.0f, 0.0f, 25.0f))
                ),
                projectileLifetime
        );
        meshRenderer.registerMesh(activeProjectiles.back().projectileEntity);
    }

    void SpaceSim::update(const long long int &deltaMicroseconds) {
        const double deltaSeconds = (double) deltaMicroseconds / 1'000'000.0;
        const double deltaSecondsSquared = deltaSeconds * deltaSeconds;

        checkHotkeys(deltaMicroseconds);
        if (_isFinished) return;
        handleFlightControls(deltaSeconds, deltaSecondsSquared);
        initializeHotkeys();

        entity::EntityRegistry &registry = entity::EntityRegistry::getInstance();

        components::ApplyVelocitySystem(registry, deltaSeconds, deltaSecondsSquared).execute();
        components::OrbitalSystem(registry, deltaSeconds, deltaSecondsSquared).execute();
        components::ApplyRotationalVelocitySystem(registry, deltaSeconds, deltaSecondsSquared).execute();
        components::ApplyScaleVelocitySystem(registry, deltaSeconds, deltaSecondsSquared).execute();

        registry.execute([&registry](const entity::EntityReference *entity, components::Light light, components::Transform transform) {
            light.setPosition(transform.getPosition());
            registry.addOrSetComponent(entity, light);
        });

        for (int i = static_cast<int>(activeProjectiles.size()) - 1; i >= 0; i--) {
            ProjectileData &projectile = activeProjectiles[i];
            projectile.remainingLifeTime -= deltaSeconds;
            if (projectile.remainingLifeTime < 0.0) {
                meshRenderer.removeMesh(projectile.projectileEntity);
                graphics::LightManager::getInstance().removeLight(projectile.projectileEntity);
                registry.eraseEntity(projectile.projectileEntity);
                delete projectile.projectileEntity;
                activeProjectiles.erase(activeProjectiles.begin() + i);
            } else {
                components::Light light = registry.getComponentData<components::Light>(projectile.projectileEntity).value();
                light.setIntensity(projectileLightIntensity * static_cast<float>(projectile.remainingLifeTime / projectileLifetime));
                registry.addOrSetComponent(projectile.projectileEntity, light);
            }
        }

        activeFollowCamera->update(deltaSeconds);
        meshRenderer.update();
        graphics::LightManager::LightSystem(registry).execute();
    }

    void SpaceSim::draw(const long long int &deltaMicroseconds) {
        meshRenderer.present(program.getID());
    }

    void SpaceSim::onResume() {
        initializeHotkeys();
        loadShaders();
        bindLighting();
        activeFollowCamera->bindCamera();
        printControls();
    }

    void SpaceSim::onPause() {
        spdlog::info("===== SpaceSim paused =====");
    }

    void SpaceSim::onExit() {
        spdlog::info("exiting SpaceSim");

        meshRenderer.clear();
        entity::EntityRegistry &registry = entity::EntityRegistry::getInstance();
        graphics::LightManager &lightManager = graphics::LightManager::getInstance();

        registry.eraseEntity(playerShipEntity);
        delete playerShipEntity;

        registry.eraseEntity(skyboxEntity);
        delete skyboxEntity;

        for (entity::EntityReference *entity: solarSystemEntities) {
            std::optional<components::Light> lightOpt = registry.getComponentData<components::Light>(entity);
            if (lightOpt.has_value()) {
                lightManager.removeLight(lightOpt.value());
            }
            registry.eraseEntity(entity);
            delete entity;
        }
        solarSystemEntities.clear();

        _isFinished = true;
    }

}