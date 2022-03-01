#ifndef ACAENGINE_LIGHTDATA_H
#define ACAENGINE_LIGHTDATA_H

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>

namespace graphics {
    enum LightType : int {
        directional = 0,
        spot = 1,
        point = 2
    };

    struct LightData {
        /**
         * Directional Light:
         *   position     -
         *   direction    direction the light is emitted in
         *   range        -
         *   spot_angle   -
         *   color        color of the emitted light
         *   intensity    simple multiplier for emitted light
         * Spot-Light:
         *   position     position of the light source in world space
         *   direction    direction the spot-light is facing
         *   range        maximum range of emitted light
         *   spot_angle   max angle the light is emitted at in degrees (e.g. 5° means light is emitted in a cone of 2.5° from the spot-light's direction)
         *   color        color of the emitted light
         *   intensity    resulting light-intensity at a given point is `intensity * ( 1 - ([point_to_light_distance] / [range]) )²`
         * Point-Light:
         *   position     position of the light source in world space
         *   direction    -
         *   range        maximum range of emitted light
         *   spot_angle   -
         *   color        color of the emitted light
         *   intensity    resulting light-intensity at a given point is `intensity * ( 1 - ([point_to_light_distance] / [range]) )²`
         */

        alignas(4) LightType _type;
        alignas(4) float _range;
        alignas(4) float _spotAngleCosine;
        alignas(4) float _intensity;
        alignas(16) glm::vec3 _position;
        alignas(16) glm::vec3 _direction;
        alignas(16) glm::vec3 _color;

        LightData() = default;

        constexpr LightData(LightType lightType, glm::vec3 lightPosition, glm::vec3 lightDirection, float lightRange, float lightSpotAngleCosine,
                            glm::vec3 lightColor, float lightIntensity) :
                _type(lightType),
                _position(lightPosition),
                _direction(lightDirection),
                _range(lightRange),
                _spotAngleCosine(lightSpotAngleCosine),
                _color(lightColor),
                _intensity(lightIntensity) {}
    };
}


#endif //ACAENGINE_LIGHTDATA_H
