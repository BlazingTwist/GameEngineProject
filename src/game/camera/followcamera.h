#ifndef ACAENGINE_FOLLOWCAMERA_H
#define ACAENGINE_FOLLOWCAMERA_H

#include <engine/graphics/camera.hpp>
#include <engine/input/inputmanager.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <engine/entity/entityregistry.h>
#include <engine/components/transform.h>
#include <spdlog/spdlog.h>

namespace game {
    class FollowCamera {
    public:
        FollowCamera(graphics::Camera _camera, glm::vec3 _initialPositionOffset, glm::vec3 _initialRotationOffset,
                     input::MouseButton _unlockCameraRotationButton, glm::vec2 _mouseSensitivity,
                     input::Key _translateForwardKey, input::Key _translateBackwardKey, input::Key _translateLeftKey, input::Key _translateRightKey,
                     input::Key _translateUpKey, input::Key _translateDownKey, glm::vec3 _translateSensitivity);

        void loadShaders(const unsigned int &programID);

        void trackEntity(entity::EntityReference *entity);

        void resetCameraOffsets();

        void bindCamera() const;

        void update(const double &deltaSeconds);

    private:
        void applyCameraTransform();
        
        bool checkTranslationInput(glm::vec3 &inputVector) const;

    public:
        graphics::Camera camera;
        GLint worldToCameraMatrixID = 0;
        GLint cameraPositionShaderID = 0;

        glm::vec3 currentPositionOffset;
        glm::vec3 currentRotationOffset;

        const glm::vec3 initialPositionOffset;
        const glm::vec3 initialRotationOffset;

        const input::MouseButton unlockCameraRotationButton;
        const glm::vec2 mouseSensitivity;

        const input::Key translateForwardKey;
        const input::Key translateBackwardKey;
        const input::Key translateUpKey;
        const input::Key translateDownKey;
        const input::Key translateLeftKey;
        const input::Key translateRightKey;
        const glm::vec3 translateSensitivity;

    private:
        entity::EntityReference *trackedEntity = nullptr;
        glm::vec3 lastEntityPosition = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::quat lastEntityRotation = glm::quat(glm::vec3(0.0f, 0.0f, 0.0f));

        bool rotationUnlocked = false;
        glm::vec2 prevCursorPos = glm::vec2(0.0f, 0.0f);

    };
}

#endif //ACAENGINE_FOLLOWCAMERA_H