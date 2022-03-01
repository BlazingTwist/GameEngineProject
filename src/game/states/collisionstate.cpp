/*#include "collisionstate.h"


namespace gameState {

    glm::vec3 xAxis(0, -1, 0);
    glm::mat4 rota(glm::rotate(glm::identity<glm::mat4>(), 1.0f, xAxis));
    std::vector<int> ids;
    static graphics::Sampler* sampler;
    double timer2 = 0;
    static constexpr auto defaultLightRange = 100.0f;
    static constexpr auto defaultLightIntensity = 0.75f;
    double timer = 0;
    glm::vec3 curserpos;
    int counter = 0;


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
            if (planet->planetID != i) {
                planet->planetID = i;
            }
            planet->planetID = meshRenderer.draw(planet->sphereMeshes,
                graphics::Texture2DManager::get("textures/planet1.png", *sampler),
                graphics::Texture2DManager::get("textures/Planet1_phong.png", *sampler),
                glm::translate(glm::identity<glm::mat4>(), planet->startposition));
            
            

         
        }
        for (int i = 0; i < bulletVec.size(); i++) {
            Bullet* bullet = &bulletVec[i];
            int j = planetVec.size() + i-1;
            bullet->bulletID = j;
            bullet->bulletID = meshRenderer.draw(bullet->sphereMeshes,
                graphics::Texture2DManager::get("textures/planet1.png", *sampler),
                graphics::Texture2DManager::get("textures/Planet1_phong.png", *sampler),
                glm::translate(glm::identity<glm::mat4>(), bullet->bullPos));
        }

    }
    void CollisionState::loadGeometry() {
    
        if ( planetVec.size()>0) {
            Planet *planet = &planetVec.back();
            
            planet->planetID = meshRenderer.draw(planet->sphereMeshes,
            graphics::Texture2DManager::get("textures/planet1.png", *sampler),
            graphics::Texture2DManager::get("textures/Planet1_phong.png", *sampler),
            glm::translate(glm::identity<glm::mat4>(), planet->startposition));
            counter++;
        }
    }
    
   
    void CollisionState::createBullet(const long long int& deltaMicroseconds) {
 
        math::AABB<3, double> aabb;
        
        Bullet bullet = { planetVec.size() + bulletVec.size(),(graphics::Mesh(utils::MeshLoader::get("models/sphere.obj"))),cameraControls.camera.getPosition(),curserpos,aabb };
        bullet.bullDirection  = curserpos - cameraControls.camera.getPosition();
        std::cout << bullet.bullPos.x;
        
        //spdlog::info(curserpos.x);

        glm::vec3 min = { bullet.bullPos.x,bullet.bullPos.y,bullet.bullPos.z };
        glm::vec3 max = { bullet.bullPos.x,bullet.bullPos.y,bullet.bullPos.z };
         

            bulletVec.push_back(bullet);
          
            spdlog::info("wie oft wird das per mausklick gemacht?");
            bullet.bulletID = meshRenderer.draw(bullet.sphereMeshes,
                graphics::Texture2DManager::get("textures/planet1.png", *sampler),
                graphics::Texture2DManager::get("textures/Planet1_phong.png", *sampler),
                glm::translate(glm::identity<glm::mat4>(), bullet.bullPos));
            counter++;
                
       
    }

    void CollisionState::initializeScene() {
        cameraControls.initializeScene();
        lightData.light_range = defaultLightRange;
        lightData.light_intensity = defaultLightIntensity;
        
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

    void CollisionState::updateBulletPos(const long long int& deltaMicroseconds)
    {
        double deltaSeconds = (double)deltaMicroseconds / 1'000'000.0;
        for (int i = 0; i < bulletVec.size();i++) {
            Bullet *bullet = &bulletVec[i];

            
            bullet->bullPos.x += bullet->bullDirection.x * deltaSeconds;
            bullet->bullPos.y += bullet->bullDirection.y * deltaSeconds;
            bullet->bullPos.z += bullet->bullDirection.z * deltaSeconds;
            glm::vec3 min = { bullet->bullPos.x,bullet->bullPos.y,bullet->bullPos.z };
            glm::vec3 max = { bullet->bullPos.x,bullet->bullPos.y,bullet->bullPos.z };
            
            bullet->aabb.min = min;
            bullet->aabb.max = max;
            
           // std::cout <<bullet->aabb.min.x;

            if (bullet->bullPos.x < -50 || bullet->bullPos.x>100) {
                bullet->bullDirection.x = -bullet->bullDirection.x;

            }
            if (bullet->bullPos.y < -50 || bullet->bullPos.y>100) {
                bullet->bullDirection.y = -bullet->bullDirection.y;
            }
            if (bullet->bullPos.z < -50 || bullet->bullPos.z>100) {
                bullet->bullDirection.z = -bullet->bullDirection.z;
            }
          meshRenderer.setTransform(bullet->bulletID, glm::translate(glm::identity<glm::mat4>(), bullet->bullPos));

        }
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

        meshRenderer(graphics::MeshRenderer()),
        sphereMash(graphics::Mesh(utils::MeshLoader::get("models/sphere.obj")))
    {
       
        spdlog::info("sief");
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
  
    void CollisionState::updatePos(const long long int& deltaMicroseconds)

    {
        double deltaSeconds = (double)deltaMicroseconds / 1'000'000.0;
         for (int i = 0; i < planetVec.size(); i++)
        {
             
                 // spdlog::info(planetVec[i].planetID != NULL);
                 Planet* planet = &planetVec[i];

                 planet->startposition.x = planet->startposition.x + planet->direction.x * deltaSeconds  ;
                 planet->startposition.y = planet->startposition.y + planet->direction.y * deltaSeconds ;
                 planet->startposition.z = planet->startposition.z + planet->direction.z * deltaSeconds ;

                 //part of AABB collision detection
                 planet->aabb.min.x = planet->startposition.x - 10;
                 planet->aabb.max.x = planet->startposition.x +10      ;
                 planet->aabb.min.y = planet->startposition.y -10;
                 planet->aabb.max.y = planet->startposition.y +10;
                 planet->aabb.min.z = planet->startposition.z -10.f;
                 planet->aabb.max.z = planet->startposition.z +10;

                 glm::vec3 ursprung(-planet->startposition.x, -planet->startposition.y, -planet->startposition.z);
                 glm::vec3 directRot = glm::normalize(glm::vec3(-planet->direction.x, -planet->direction.y, -planet->direction.z));

                 planet->angularVelocity = planet->angularVelocity + deltaSeconds;

                 //(glm::translate(glm::identity<glm::mat4>(), ursprung)
                 meshRenderer.setTransform(planet->planetID, (glm::translate(glm::rotate(glm::identity<glm::mat4>(),
                     planet->angularVelocity, planet->direction), planet->startposition)));
                 //Box for planets to stay in, probably better to change the scene.
                 if (planet->startposition.x > 40 || planet->startposition.x <= 0) {
                     planet->direction.x = -planet->direction.x;
                     

                 }
                 else if (planet->startposition.y > 40 || planet->startposition.y <= 0) {
                     planet->direction.y = -planet->direction.y;


                 }
                 else if (planet->startposition.z > 40 || planet->startposition.z <= 0) {
                     planet->direction.z = -planet->direction.z;
                 }
        }
    }


    void CollisionState::createPlanets(const long long& deltaMicroseconds)
    {

        
        if (planetVec.size() < 20 &&timer>=1){
            timer = 0;
         
            glm::vec3 pos(randomArray[counter * 3], randomArray[counter * 3 + 1], randomArray[counter * 3 + 2]);
            glm::vec3 direct(randomArray2[counter * 3], randomArray2[counter * 3 + 1], randomArray2[counter * 3 + 2]);
            glm::vec3 min(pos.x - 0.1, pos.y - 0.1, pos.z - 0.1);
            glm::vec3 max(pos.x + 0.1, pos.y + 0.1, pos.z + 0.1);
            float why = randomArray[counter];
            float angVel = glm::radians(why);

            Planet planet = { planetVec.size()+bulletVec.size(),(graphics::Mesh(utils::MeshLoader::get("models/sphere.obj"))),pos,direct,0.f,angVel,{planet.aabb} }; 
            planet.aabb.min = min;
            planet.aabb.max = max;
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
           
            utils::SparseOctree<Planet, 3, double> tree(1.f);
            for (int i = 0; i < planetVec.size(); i++) {
                Planet *planet = &planetVec[i];
                tree.insert(planet->aabb, *planet);

            }
            for (Bullet bullet : bulletVec) {

                TreeProcessor treeProc(bullet);
                if (planetVec.size() > 0) {
                    tree.traverse(treeProc);
                }
                tree.clear();
                for (int i = 0; i < treeProc.fits.size(); i++)
                {


                    for (int j = 0; j < planetVec.size(); j++)
                    {
                        if (treeProc.fits[i].planetID == planetVec[j].planetID) {
                            std::cout << planetVec[j].planetID << " got deleted";
                            planetVec.erase(planetVec.begin() + j);
//                            std::cout << planetVec[j].planetID << " got deleted";
                            ids.push_back(j);
                            j--;    
                            meshRenderer.clear();
                            reloadGeometry();
                         
                        }

                    }
                  
                }  treeProc.fits.clear();
    
            }
            //draw(deltaMicroseconds);
        createPlanets(deltaMicroseconds);
        updatePos( deltaMicroseconds);
        updateBulletPos(deltaMicroseconds);
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