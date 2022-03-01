
#include "light.h"

namespace components {

    const glm::vec3 &components::Light::getPosition() const {
        return position;
    }

    void Light::setPosition(const glm::vec3 &_position) {
        position = _position;
        lightDataChanged = true;
    }

    const glm::vec3 &Light::getDirection() const {
        return direction;
    }

    void Light::setDirection(const glm::vec3 &_direction) {
        direction = glm::normalize(_direction);
        lightDataChanged = true;
    }

    float Light::getRange() const {
        return range;
    }

    void Light::setRange(float _range) {
        range = _range;
        lightDataChanged = true;
    }

    float Light::getSpotAngle() const {
        return spotAngle;
    }

    void Light::setSpotAngle(float _spotAngle) {
        spotAngle = _spotAngle;
        _spotAngleCosine = glm::cos(glm::radians(_spotAngle));
        lightDataChanged = true;
    }

    const glm::vec3 &Light::getColor() const {
        return color;
    }

    void Light::setColor(const glm::vec3 &_color) {
        color = _color;
        lightDataChanged = true;
    }

    float Light::getIntensity() const {
        return intensity;
    }

    void Light::setIntensity(float _intensity) {
        intensity = _intensity;
        lightDataChanged = true;
    }
}
