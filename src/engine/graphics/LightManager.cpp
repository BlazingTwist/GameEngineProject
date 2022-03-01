#include "LightManager.h"

namespace graphics {

    void LightManager::LightSystem::execute() {
        bool boundLightCountChanged = false;
        std::vector<const entity::EntityReference *> changedLights = {};
        // make sure all Lights are registered in LightManager
        registry.execute([this, &boundLightCountChanged, &changedLights](const entity::EntityReference *entity, components::Light light) {
            if (light.getLightManagerId() < 0) {
                light.setLightManagerId(static_cast<int>(lightManager.boundLightCount));
                lightManager.boundLightCount++;
                boundLightCountChanged = true;
                lightManager.boundLights.push_back(entity);
                registry.addOrSetComponent(entity, light);
            }
            if (light.isLightDataChanged()) {
                changedLights.push_back(entity);
            }
        });

        bool boundLightsChanged = !changedLights.empty();
        if (boundLightCountChanged || boundLightsChanged) {
            glCall(glBindBuffer, GL_SHADER_STORAGE_BUFFER, lightManager.lightSSBO);
            bool bufferWasResized = false;
            if (boundLightCountChanged) {
                if (lightManager.boundLightCount > lightManager.currentPossibleBoundLightCount) {
                    bufferWasResized = true;
                    lightManager.performResize();
                }
                glCall(glBufferSubData, GL_SHADER_STORAGE_BUFFER, 0, sizeof(unsigned int), &lightManager.boundLightCount);
            }
            if (!bufferWasResized && boundLightsChanged) {
                for (const entity::EntityReference *changedEntity: changedLights) {
                    components::Light changedLight = registry.getComponentData<components::Light>(changedEntity).value();
                    graphics::LightData lightData = changedLight.getLightData();
                    glCall(glBufferSubData, GL_SHADER_STORAGE_BUFFER,
                           LightCountField_ByteOffset + (changedLight.getLightManagerId() * sizeof(graphics::LightData)),
                           sizeof(graphics::LightData), &lightData);
                    changedLight.lightDataChangeHandled();
                    registry.addOrSetComponent(changedEntity, changedLight);
                }
            }
        }
    }

    void LightManager::removeLight(const entity::EntityReference *lightEntity) {
        removeLight(entity::EntityRegistry::getInstance().getComponentData<components::Light>(lightEntity).value());
    }

    void LightManager::removeLight(const components::Light &light) {
        if (boundLightCount == 0) {
            spdlog::error("tried to remove light, but LightManager is empty!");
            return;
        }

        boundLightCount--;
        glCall(glBindBuffer, GL_SHADER_STORAGE_BUFFER, lightSSBO);
        glCall(glBufferSubData, GL_SHADER_STORAGE_BUFFER, 0, sizeof(unsigned int), &boundLightCount);
        if (light.getLightManagerId() != boundLightCount) {
            const entity::EntityReference *moveEntity = boundLights.back();
            boundLights[light.getLightManagerId()] = moveEntity;

            components::Light movedLight = entity::EntityRegistry::getInstance().getComponentData<components::Light>(moveEntity).value();
            movedLight.setLightManagerId(light.getLightManagerId());
            entity::EntityRegistry::getInstance().addOrSetComponent(moveEntity, movedLight);
            graphics::LightData lightData = movedLight.getLightData();
            glCall(glBufferSubData, GL_SHADER_STORAGE_BUFFER, LightCountField_ByteOffset + (movedLight.getLightManagerId() * sizeof(graphics::LightData)),
                   sizeof(graphics::LightData), &lightData);
        }
        boundLights.pop_back();
    }

    void LightManager::performResize() {
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

}