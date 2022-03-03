#include "followcamera.h"

namespace game {

    FollowCamera::FollowCamera(graphics::Camera _camera, glm::vec3 _initialPositionOffset, glm::vec3 _initialRotationOffset,
                               input::MouseButton _unlockCameraRotationButton, glm::vec2 _mouseSensitivity,
                               input::Key _translateForwardKey, input::Key _translateBackwardKey, input::Key _translateLeftKey, input::Key _translateRightKey,
                               input::Key _translateUpKey, input::Key _translateDownKey, glm::vec3 _translateSensitivity)
            : camera(_camera),
              initialPositionOffset(_initialPositionOffset), currentPositionOffset(_initialPositionOffset),
              initialRotationOffset(_initialRotationOffset), currentRotationOffset(_initialRotationOffset),
              unlockCameraRotationButton(_unlockCameraRotationButton), mouseSensitivity(_mouseSensitivity),
              translateForwardKey(_translateForwardKey), translateBackwardKey(_translateBackwardKey), translateUpKey(_translateUpKey),
              translateDownKey(_translateDownKey), translateLeftKey(_translateLeftKey), translateRightKey(_translateRightKey),
              translateSensitivity(_translateSensitivity) {}

    void FollowCamera::loadShaders(const unsigned int &programID) {
        worldToCameraMatrixID = glGetUniformLocation(programID, "world_to_camera_matrix");
        cameraPositionShaderID = glGetUniformLocation(programID, "camera_position");
    }

    void FollowCamera::trackEntity(entity::EntityReference *entity) {
        trackedEntity = entity;
    }

    void FollowCamera::resetCameraOffsets() {
        currentPositionOffset = initialPositionOffset;
        currentRotationOffset = initialRotationOffset;
        applyCameraTransform();
    }

    void FollowCamera::bindCamera() const {
        glUniformMatrix4fv(worldToCameraMatrixID, 1, GL_FALSE, glm::value_ptr(camera.getWorldToCamera()));
        glUniform3fv(cameraPositionShaderID, 1, glm::value_ptr(camera.getPosition()));
    }

    void FollowCamera::update(const double &deltaSeconds) {
        bool unlockRotationPressed = input::InputManager::isButtonPressed(unlockCameraRotationButton);
        if (rotationUnlocked) {
            if (unlockRotationPressed) {
                glm::vec2 currentCursorPos = input::InputManager::getCursorPos();
                glm::vec2 rotationDelta = (currentCursorPos - prevCursorPos) * mouseSensitivity * static_cast<float>(deltaSeconds);
                prevCursorPos = currentCursorPos;
                currentRotationOffset += glm::vec3(glm::radians(rotationDelta.y), glm::radians(rotationDelta.x), 0.0f);
            } else {
                rotationUnlocked = false;
            }
        } else if (unlockRotationPressed) {
            rotationUnlocked = true;
            prevCursorPos = input::InputManager::getCursorPos();
        }

        glm::vec3 translateInput = glm::vec3(0.0f, 0.0f, 0.0f);
        if (checkTranslationInput(translateInput)) {
            currentPositionOffset += glm::quat(currentRotationOffset) * (translateInput * static_cast<float>(deltaSeconds));
        }

        if (trackedEntity != nullptr) {
            entity::EntityRegistry &registry = entity::EntityRegistry::getInstance();
            std::optional<components::Transform> entityTransformOpt = registry.getComponentData<components::Transform>(trackedEntity);
            if (!entityTransformOpt.has_value()) {
                spdlog::error("tracked entity does not have a transform component!");
                return;
            }

            components::Transform &entityTransform = entityTransformOpt.value();
            lastEntityPosition = entityTransform.getPosition();
            lastEntityRotation = entityTransform.getRotation();
        }

        applyCameraTransform();
        bindCamera();
    }

    void FollowCamera::applyCameraTransform() {
        camera.setPosition(lastEntityPosition + (lastEntityRotation * currentPositionOffset));
        camera.setRotation(glm::toMat3(lastEntityRotation * glm::quat(currentRotationOffset)));
    }

    bool FollowCamera::checkTranslationInput(glm::vec3 &inputVector) const {
        bool doTranslate = false;
        if (input::InputManager::isKeyPressed(translateForwardKey)) {
            inputVector.z += translateSensitivity.z;
            doTranslate = true;
        }
        if (input::InputManager::isKeyPressed(translateBackwardKey)) {
            inputVector.z -= translateSensitivity.z;
            doTranslate = true;
        }
        if (input::InputManager::isKeyPressed(translateLeftKey)) {
            inputVector.x -= translateSensitivity.x;
            doTranslate = true;
        }
        if (input::InputManager::isKeyPressed(translateRightKey)) {
            inputVector.x += translateSensitivity.x;
            doTranslate = true;
        }
        if (input::InputManager::isKeyPressed(translateUpKey)) {
            inputVector.y += translateSensitivity.y;
            doTranslate = true;
        }
        if (input::InputManager::isKeyPressed(translateDownKey)) {
            inputVector.y -= translateSensitivity.y;
            doTranslate = true;
        }
        return doTranslate;
    }
}