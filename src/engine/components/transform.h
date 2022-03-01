#ifndef ACAENGINE_TRANSFORM_H
#define ACAENGINE_TRANSFORM_H

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

namespace components {

    /**
     * Describes the position, rotation and scale of an Object in the world.
     * 
     * Changes to this transform should be made during the update tick.
     * Changes are then managed and cached during the draw tick.
     */
    class Transform {

    public:
        Transform() {}

        Transform(const glm::vec3 &position, const glm::quat &rotation, const glm::vec3 &scale) : _position(position), _rotation(rotation), _scale(scale) {
            updateTransformMatrix();
        }

        [[nodiscard]] const glm::vec3 &getPosition() const {
            return _position;
        }

        void setPosition(const glm::vec3 &position) {
            _position = position;
            updateTransformMatrix();
        }

        [[nodiscard]] const glm::quat &getRotation() const {
            return _rotation;
        }

        void setRotation(const glm::quat &rotation) {
            _rotation = rotation;
            updateTransformMatrix();
        }

        [[nodiscard]] const glm::vec3 &getScale() const {
            return _scale;
        }

        void setScale(const glm::vec3 &scale) {
            _scale = scale;
            updateTransformMatrix();
        }

        [[nodiscard]] const glm::mat4 &getTransformMatrix() const {
            return transformMatrix;
        }

        [[nodiscard]] bool hasTransformChanged() const {
            return transformChanged;
        }

        /**
         * This method should be called at the end of the draw tick.
         */
        void onChangesHandled() {
            transformChanged = false;
        }

    private:
        void updateTransformMatrix() {
            transformMatrix = glm::translate(glm::identity<glm::mat4>(), _position) * glm::toMat4(_rotation) * glm::scale(glm::identity<glm::mat4>(), _scale);
            transformChanged = true;
        }

        glm::vec3 _position;
        glm::quat _rotation;
        glm::vec3 _scale;

        glm::mat4 transformMatrix;
        bool transformChanged = false;
    };
}

#endif //ACAENGINE_TRANSFORM_H
