#ifndef ACAENGINE_DEFAULTCAMERACONTROLS_H
#define ACAENGINE_DEFAULTCAMERACONTROLS_H

#include <engine/graphics/camera.hpp>
#include <engine/input/inputmanager.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>

namespace game {
    class DefaultCameraControls {
    public:
        DefaultCameraControls(graphics::Camera camera, glm::vec3 initialCameraPosition, float initialCameraPitch, float initialCameraYaw,
                              float initialCameraRoll);

        graphics::Camera camera;
        GLint worldToCameraMatrixID = 0;
        GLint cameraPositionShaderID = 0;

        glm::vec2 prevCursorPos = glm::vec2(0.0f, 0.0f);
        float cameraPitch = 0.0f;
        float cameraYaw = 0.0f;
        float cameraRoll = 0.0f;

        void initializeCursorPosition();

        void loadShaders(const unsigned int &programID);

        void initializeScene();

        void bindCamera() const;

        void update(const long long &deltaMicroseconds);

    private:
        const glm::vec3 initialCameraPosition;
        const float initialCameraPitch;
        const float initialCameraYaw;
        const float initialCameraRoll;
    };
}

#endif //ACAENGINE_DEFAULTCAMERACONTROLS_H
