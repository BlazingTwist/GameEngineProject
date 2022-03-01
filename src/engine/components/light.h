#ifndef ACAENGINE_LIGHT_H
#define ACAENGINE_LIGHT_H

#include <engine/graphics/lightdata.h>

namespace components {
    class Light {
    public:
        Light() : lightData(graphics::LightData::point(glm::vec3(0.0f, 0.0f, 0.0f), 75.0f, glm::vec3(1.0f, 1.0f, 0.8f), 0.75f)), lightManagerID(-1) {}

        explicit Light(const graphics::LightData &_lightData) : lightData(_lightData), lightManagerID(-1) {}

        [[nodiscard]] const graphics::LightData &getLightData() const {
            return lightData;
        }

        void setLightData(const graphics::LightData &_lightData) {
            Light::lightData = _lightData;
            Light::lightDataChanged = true;
        }

        [[nodiscard]] int getLightManagerId() const {
            return lightManagerID;
        }

        void setLightManagerId(int _lightManagerId) {
            Light::lightManagerID = _lightManagerId;
        }

        [[nodiscard]] bool isLightDataChanged() const {
            return lightDataChanged;
        }

        void lightDataChangeHandled() {
            Light::lightDataChanged = false;
        }

    private:
        graphics::LightData lightData;
        int lightManagerID;
        bool lightDataChanged = true;
    };
}

#endif //ACAENGINE_LIGHT_H
