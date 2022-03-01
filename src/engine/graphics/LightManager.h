#ifndef ACAENGINE_LIGHTMANAGER_H
#define ACAENGINE_LIGHTMANAGER_H

#include <vector>
#include <engine/components/light.h>
#include <engine/entity/entityregistry.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>
#include "core/opengl.hpp"

namespace graphics {
    static constexpr int LightCountField_ByteOffset = 16; // don't ask why, I'm not sure either, has something to do with OpenGLs memory alignment rules.

    class LightManager {
    public:
        class LightSystem {
        public:
            explicit LightSystem(entity::EntityRegistry &_registry) : registry(_registry), lightManager(LightManager::getInstance()) {}

            void execute();

        private:
            entity::EntityRegistry &registry;
            LightManager &lightManager;
        };


        static LightManager &getInstance() {
            static LightManager instance;
            return instance;
        }

        void removeLight(const entity::EntityReference *light);
        
        void removeLight(const components::Light &light);

        void bindLights(int bindIndex) const {
            glCall(glBindBuffer, GL_SHADER_STORAGE_BUFFER, lightSSBO);
            glCall(glBindBufferBase, GL_SHADER_STORAGE_BUFFER, bindIndex, lightSSBO);
        }

        LightManager(LightManager const &) = delete;

        void operator=(LightManager const &) = delete;

        virtual ~LightManager() {
            for (auto *item: boundLights) {
                delete item;
            }
            boundLights.clear();
        }

    private:
        LightManager() : lightSSBO(0) {
            glCall(glGenBuffers, 1, &lightSSBO);
            glCall(glBindBuffer, GL_SHADER_STORAGE_BUFFER, lightSSBO);
            //glCall(glBufferData, GL_SHADER_STORAGE_BUFFER, sizeof(unsigned int) + sizeof(graphics::LightData), nullptr, GL_DYNAMIC_DRAW);
            glCall(glBufferData, GL_SHADER_STORAGE_BUFFER, LightCountField_ByteOffset, nullptr, GL_DYNAMIC_DRAW);
            glCall(glBufferSubData, GL_SHADER_STORAGE_BUFFER, 0, sizeof(unsigned int), &boundLightCount);
        };

        void performResize() {
            currentPossibleBoundLightCount = static_cast<unsigned int>(static_cast<float>(boundLightCount + 1) * 1.3f);
            const unsigned int lightDataByteSize = currentPossibleBoundLightCount * sizeof(graphics::LightData);
            glCall(glBufferData, GL_SHADER_STORAGE_BUFFER,
                   LightCountField_ByteOffset + lightDataByteSize,
                   nullptr, GL_DYNAMIC_DRAW);

            auto *lights = new LightData[boundLightCount];
            entity::EntityRegistry &registry = entity::EntityRegistry::getInstance();
            for (unsigned int i = 0; i < boundLightCount; i++) {
                components::Light lightComp = registry.getComponentData<components::Light>(boundLights[i]).value();
                lights[i] = lightComp.getLightData();
                if (lightComp.isLightDataChanged()) {
                    lightComp.lightDataChangeHandled();
                    registry.addOrSetComponent(boundLights[i], lightComp);
                }
            }
            glCall(glBufferSubData, GL_SHADER_STORAGE_BUFFER, LightCountField_ByteOffset, static_cast<int>(lightDataByteSize), lights);
            delete[] lights;
        }

        GLuint lightSSBO;
        unsigned int boundLightCount = 0;
        unsigned int currentPossibleBoundLightCount = 0;
        std::vector<const entity::EntityReference *> boundLights = {};

    };
}

#endif //ACAENGINE_LIGHTMANAGER_H
