#ifndef ACAENGINE_LIGHT_H
#define ACAENGINE_LIGHT_H

#include <engine/graphics/lightdata.h>

namespace components {
    class Light {
    public:
        Light() = default;

        [[nodiscard]] static Light directional(glm::vec3 direction, glm::vec3 lightColor, float lightIntensity) {
            Light result;
            result._type = graphics::LightType::directional;
            result.direction = glm::normalize(direction);
            result.color = lightColor;
            result.intensity = lightIntensity;
            return result;
        }

        [[nodiscard]] static Light spot(glm::vec3 position, glm::vec3 direction, float range, float spotAngle, glm::vec3 lightColor, float lightIntensity) {
            Light result;
            result._type = graphics::LightType::spot;
            result.position = position;
            result.direction = glm::normalize(direction);
            result.range = range;
            result.spotAngle = spotAngle;
            result._spotAngleCosine = glm::cos(glm::radians(spotAngle));
            result.color = lightColor;
            result.intensity = lightIntensity;
            return result;
        }

        [[nodiscard]] static Light point(glm::vec3 position, float range, glm::vec3 lightColor, float lightIntensity) {
            Light result;
            result._type = graphics::LightType::point;
            result.position = position;
            result.range = range;
            result.color = lightColor;
            result.intensity = lightIntensity;
            return result;
        }

        [[nodiscard]] constexpr graphics::LightData getLightData() const {
            return {_type, position, direction, range, _spotAngleCosine, color, intensity};
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

        [[nodiscard]] const glm::vec3 &getPosition() const;

        void setPosition(const glm::vec3 &_position);

        [[nodiscard]] const glm::vec3 &getDirection() const;

        void setDirection(const glm::vec3 &_direction);

        [[nodiscard]] float getRange() const;

        void setRange(float _range);

        [[nodiscard]] float getSpotAngle() const;

        void setSpotAngle(float _spotAngle);

        [[nodiscard]] const glm::vec3 &getColor() const;

        void setColor(const glm::vec3 &_color);

        [[nodiscard]] float getIntensity() const;

        void setIntensity(float _intensity);

    private:
        int lightManagerID = -1;
        bool lightDataChanged = true;
        float _spotAngleCosine = 0.0f;

        graphics::LightType _type = graphics::LightType::directional;
        glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 direction = glm::vec3(0.0f, 0.0f, 0.0f);
        float range = 0.0f;
        float spotAngle = 0.0f;
        glm::vec3 color = glm::vec3(0.0f, 0.0f, 0.0f);
        float intensity = 0.0f;
    };
}

#endif //ACAENGINE_LIGHT_H
