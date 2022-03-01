/*#include "collisionstate.h"


namespace gameState {

   
    static graphics::Sampler* sampler;
    double timer2 = 0;
    static constexpr auto defaultLightRange = 100.0f;
    static constexpr auto defaultLightIntensity = 0.75f;
    double timer = 0;
    glm::vec3 curserpos;
    int counter = 0;
    static constexpr auto defaultPlanetScale = glm::vec3(1.0f, 1.0f, 1.0f);
    static constexpr auto defaultAngularVelocity = 10;
    static constexpr auto defaultAngularVelocityBullet = 0;
    utils::SparseOctree<const entity::EntityReference*, 3, double> tree(1.f);

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

    


    void CollisionState::initializeHotkeys() {
        hotkey_reset_isDown = input::InputManager::isKeyPressed(input::Key::Num1);
        hotkey_mainState_isDown = input::InputManager::isKeyPressed(input::Key::Num2);
        hotkey_exit_isDown = input::InputManager::isKeyPressed(input::Key::Num3);
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
  
    
    void CollisionState::reloadGeometry() {
        for (int i = 0; i < planetVec.size(); i++)
        {
            Planet* planet = &planetVec[i];
            

            planet->planetID = entity::EntityRegistry::getInstance().createEntity(
                components::Transform(planet->startposition,
                    glm::quat(glm::vec3(0.0f, 0.0f, 0.0f)),
                    defaultPlanetScale),
                components::Mesh(meshRenderer.requestNewMesh(),
                    utils::MeshLoader::get("models/sphere.obj"),
                    graphics::Texture2DManager::get("textures/planet1.png", *sampler),
                    graphics::Texture2DManager::get("textures/Planet1_phong.png", *sampler)),
                components::PhysicsObject(planet->direction,defaultAngularVelocity, { planet->startposition, planet->startposition })
                );

         
        }
        for (int i = 0; i < bulletVec.size(); i++) {
            Bullet* bullet = &bulletVec[i];

            bullet->bulletID = entity::EntityRegistry::getInstance().createEntity(
                components::Transform(bullet->bullPos,
                    glm::quat(glm::vec3(0.0f, 0.0f, 0.0f)),
                    defaultPlanetScale),
                components::Mesh(meshRenderer.requestNewMesh(),
                    utils::MeshLoader::get("models/sphere.obj"),
                    graphics::Texture2DManager::get("textures/planet1.png", *sampler),
                    graphics::Texture2DManager::get("textures/Planet1_phong.png", *sampler)));
            components::PhysicsObject(bullet->bullDirection,defaultAngularVelocityBullet, { bullet->bullPos, bullet->bullPos });
        }

    }
    void CollisionState::loadGeometry() {
    
        if ( planetVec.size()>0) {
            Planet *planet = &planetVec.back();
            
            planet->planetID = entity::EntityRegistry::getInstance().createEntity(
                components::Transform(planet->startposition,
                    glm::quat(glm::vec3(0.0f, 0.0f, 0.0f)),
                    defaultPlanetScale),
                components::Mesh(meshRenderer.requestNewMesh(),
                    utils::MeshLoader::get("models/sphere.obj"),
                    graphics::Texture2DManager::get("textures/planet1.png", *sampler),
                    graphics::Texture2DManager::get("textures/Planet1_phong.png", *sampler)),
                components::PhysicsObject(planet->direction, defaultAngularVelocity, { planet->startposition, planet->startposition })

            );

            counter++;
        }
    }
    
   
    void CollisionState::createBullet(const long long int& deltaMicroseconds) {
 
        
        
        Bullet bullet = { nullptr,cameraControls.camera.getPosition(),curserpos };
        bullet.bullDirection  = curserpos - cameraControls.camera.getPosition();
        std::cout << bullet.bullDirection.x;
        


            
          
            spdlog::info("wie oft wird das per mausklick gemacht?");

            bullet.bulletID = entity::EntityRegistry::getInstance().createEntity(
                components::Transform(bullet.bullPos,
                    glm::quat(glm::vec3(0.0f, 0.0f, 0.0f)),
                    defaultPlanetScale),
                components::Mesh(meshRenderer.requestNewMesh(),
                    utils::MeshLoader::get("models/sphere.obj"),
                    graphics::Texture2DManager::get("textures/planet1.png", *sampler),
                    graphics::Texture2DManager::get("textures/Planet1_phong.png", *sampler)),
                components::PhysicsObject(bullet.bullDirection, 0, { bullet.bullPos,bullet.bullPos }));
              
            bulletVec.push_back(bullet);
         
    }

    void CollisionState::initializeScene() {
        cameraControls.initializeScene();
        lightData.light_range = defaultLightRange;
        lightData.light_intensity = defaultLightIntensity;

        entity::EntityRegistry& registry = entity::EntityRegistry::getInstance();

       
        srand(time(NULL));
        //randomNumbers used for random spawnPoints need to be adapted if more planets are needed
        for (int i = 0; i <100; i++)
        {
            randomArray[i] = rand() %10;

        }
        //random Numbers for the initial direction of the movement.
        for (int i = 0; i < 100; i++)
        {
            randomArray2[i] = rand() % 2;
        }
    }

    void CollisionState::bindLighting() {
        lightData.bindData(program.getID());
        glUniform3fv(glsl_ambient_light, 1, glm::value_ptr(ambientLightData));

    }
  
    CollisionState::CollisionState() :
        cameraControls(graphics::Camera(90.0f, 0.1f, 300.0f), glm::vec3(0.0f, 0.0f, -7.0f), 0.0f, 0.0f, 0.0f),
        ambientLightData({ 1.4f, 1.4f, 1.4f }),
        lightData(graphics::LightData::point(
            glm::vec3(0.0f, 0.0f, 0.0f),
            defaultLightRange,
            glm::vec3(1.0f, 1.0f, 0.8f),
            defaultLightIntensity
        )),

        meshRenderer(graphics::MeshRenderer())

    {
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
        cameraControls.bindCamera();;
    }
  
  



    void CollisionState::createPlanets(const long long& deltaMicroseconds)
    {

        
        if (planetVec.size() < 20 &&timer>=1){
            timer = 0;
         
            glm::vec3 pos(randomArray[counter * 3], randomArray[counter * 3 + 1], randomArray[counter * 3 + 2]);
            glm::vec3 direct(randomArray2[counter * 3], randomArray2[counter * 3 + 1], randomArray2[counter * 3 + 2]);
            float why = randomArray[counter];
            float angVel = glm::radians(why);
            Planet planet = { nullptr ,pos,direct,0.f,angVel };
            planetVec.push_back(planet);
            loadGeometry();
            
          
        }
    }
    void CollisionState::update(const long long& deltaMicroseconds) {
        double deltaSeconds = (double)deltaMicroseconds / 1'000'000.0;
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
            spdlog::info("exiting collisionstate state");
            _isFinished = true;
            return;
        }

        timer = timer + (deltaSeconds);


        entity::EntityRegistry::getInstance().execute(
            [deltaSeconds](const entity::EntityReference* entity, components::Transform transform,
                components::PhysicsObject phys) {
                    

                    spdlog::info(transform.getPosition().x);
                    transform.setPosition(transform.getPosition() + (phys._velocity) * (float)deltaSeconds);
                    if (entity::EntityRegistry::getInstance().getComponentData<components::PhysicsObject>(entity).value()._angularVelocity != 0) {
                        tree.insert(phys._aabb, entity);


                        transform.setRotation(transform.getPosition() + glm::vec3(12,22,7));
                    }
                    phys._aabb.min = glm::vec3(transform.getPosition().x - 1, transform.getPosition().y - 1, transform.getPosition().z - 1);
                    phys._aabb.max = glm::vec3(transform.getPosition().x + 1, transform.getPosition().y + 1, transform.getPosition().z + 1);
                    if (phys._velocity.x < -40 || phys._velocity.x>40)
                        phys._velocity.x = -phys._velocity.x;
                    if (phys._velocity.y < -40 || phys._velocity.y>40)
                        phys._velocity.y = -phys._velocity.y;
                    if (phys._velocity.z < -40 || phys._velocity.z>40)
                        phys._velocity.z = -phys._velocity.z;
                    std::cout << phys._aabb.min.x;

                    entity::EntityRegistry::getInstance().addOrSetComponent(entity, transform);
                    entity::EntityRegistry::getInstance().addOrSetComponent(entity, phys);
                    if (entity::EntityRegistry::getInstance().getComponentData<components::PhysicsObject>(entity).value()._angularVelocity != 0) {
                        tree.insert(phys._aabb, entity);
                        
                    }

            });

        for (int i = 0; i < bulletVec.size(); i++)
        {
            Bullet* bullet = &bulletVec[i];
            if (entity::EntityRegistry::getInstance().getComponentData<components::PhysicsObject>(bullet->bulletID).value()._aabb.min.x != NULL) {

                //auto aabb = entity::EntityRegistry::getInstance().getComponentData<components::PhysicsObject>(bullet->bulletID).value()._aabb;
                TreeProcessor treeProc({ entity::EntityRegistry::getInstance().getComponentData<components::PhysicsObject>(bullet->bulletID).value()._aabb.min,
                    entity::EntityRegistry::getInstance().getComponentData<components::PhysicsObject>(bullet->bulletID).value()._aabb.max });
                if (planetVec.size() > 0)
                   // std::cout << "pla";
                tree.traverse(treeProc);
            }
        }
        tree.clear();
    
                //entity::EntityRegistry::getInstance().getComponentData( const( planet->planetID ));
               

           
                // tree.remove(aabb, planet.planetID);
                //tree.clear();
               
                //reeProc.fits.clear();
    
           
        createPlanets(deltaMicroseconds);
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
        timer2 = timer2 + deltaSeconds;
        if (input::InputManager::isButtonPressed(input::MouseButton::LEFT)&&timer2>1) {
            glm::vec2 fakecurserpos = input::InputManager::getCursorPos();
            timer2 = 0;
           
            curserpos = cameraControls.camera.toWorldSpace(fakecurserpos);
            std::cout << curserpos.z;
        
            createBullet(deltaMicroseconds);  
        }
    }
    
    void CollisionState::draw(const long long& deltaMicroseconds) {
        auto& registry = entity::EntityRegistry::getInstance();
        registry.execute([this, &registry](const entity::EntityReference* entity, components::Mesh mesh, components::Transform transform) {
            bool meshChanged = mesh.hasAnyChanges();
            bool transformChanged = transform.hasTransformChanged();
            meshRenderer.draw(mesh, transform);
            if (meshChanged) {
                registry.addOrSetComponent(entity, mesh);
            }
            if (transformChanged) {
                registry.addOrSetComponent(entity, transform);
            }
            });

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
}


     */