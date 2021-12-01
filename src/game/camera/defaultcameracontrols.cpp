#include "defaultcameracontrols.h"

namespace game {
    DefaultCameraControls::DefaultCameraControls(graphics::Camera camera, const glm::vec3 initialCameraPosition, const float initialCameraPitch,
                                                 const float initialCameraYaw, const float initialCameraRoll) :
            camera(camera),
            initialCameraPosition(initialCameraPosition),
            initialCameraPitch(initialCameraPitch),
            initialCameraYaw(initialCameraYaw),
            initialCameraRoll(initialCameraRoll) {}

    void DefaultCameraControls::initializeCursorPosition() {
        prevCursorPos = input::InputManager::getCursorPos();
    }

    void DefaultCameraControls::loadShaders(const unsigned int &programID) {
        worldToCameraMatrixID = glGetUniformLocation(programID, "world_to_camera_matrix");
        cameraPositionShaderID = glGetUniformLocation(programID, "camera_position");
    }

    void DefaultCameraControls::initializeScene() {
        camera.setPosition(initialCameraPosition);
        cameraPitch = initialCameraPitch;
        cameraYaw = initialCameraYaw;
        cameraRoll = initialCameraRoll;
        camera.setRotation(cameraYaw, cameraPitch, cameraRoll);
    }

    void DefaultCameraControls::bindCamera() const {
        glUniformMatrix4fv(worldToCameraMatrixID, 1, GL_FALSE, glm::value_ptr(camera.getWorldToCamera()));
        glUniform3fv(cameraPositionShaderID, 1, glm::value_ptr(camera.getPosition()));
    }

    void DefaultCameraControls::update(const long long &deltaMicroseconds) {
        static constexpr float cameraStep = 0.1f;
        static constexpr float cameraPitchSensitivity = 0.5f;
        static constexpr float cameraYawSensitivity = 0.5f;
        float rightInput = 0.0f;
        float forwardInput = 0.0f;
        float upInput = 0.0f;
        bool cameraPositionChanged = false;
        if (input::InputManager::isKeyPressed(input::Key::W)) {
            cameraPositionChanged = true;
            forwardInput += cameraStep;
        }
        if (input::InputManager::isKeyPressed(input::Key::S)) {
            cameraPositionChanged = true;
            forwardInput -= cameraStep;
        }
        if (input::InputManager::isKeyPressed(input::Key::D)) {
            cameraPositionChanged = true;
            rightInput += cameraStep;
        }
        if (input::InputManager::isKeyPressed(input::Key::A)) {
            cameraPositionChanged = true;
            rightInput -= cameraStep;
        }
        if (input::InputManager::isKeyPressed(input::Key::E)) {
            cameraPositionChanged = true;
            upInput += cameraStep;
        }
        if (input::InputManager::isKeyPressed(input::Key::Q)) {
            cameraPositionChanged = true;
            upInput -= cameraStep;
        }
        if (cameraPositionChanged) {
            camera.moveRelative(rightInput, upInput, forwardInput);
        }

        glm::vec2 currentCursorPos = input::InputManager::getCursorPos();
        float cursorDeltaX = currentCursorPos.x - prevCursorPos.x;
        float cursorDeltaY = currentCursorPos.y - prevCursorPos.y;
        prevCursorPos = currentCursorPos;
        bool cameraDirectionChanged = false;
        if (cursorDeltaX != 0.0f) {
            cameraDirectionChanged = true;
            cameraYaw += cursorDeltaX * cameraYawSensitivity;
            if (cameraYaw > 180.0f) {
                cameraYaw -= 360.0f;
            } else if (cameraYaw < -180.0f) {
                cameraYaw += 360.0f;
            }
        }
        if (cursorDeltaY != 0.0f) {
            cameraDirectionChanged = true;
            cameraPitch += cursorDeltaY * cameraPitchSensitivity;
            if (cameraPitch > 90.0f) {
                cameraPitch = 90.0f;
            } else if (cameraPitch < -90.0f) {
                cameraPitch = -90.0f;
            }
        }
        if (cameraDirectionChanged) {
            camera.setRotation(cameraYaw, cameraPitch, cameraRoll);
        }

        if (cameraPositionChanged || cameraDirectionChanged) {
            bindCamera();
        }
    }
}
