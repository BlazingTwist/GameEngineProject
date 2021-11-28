#pragma once

#include "../core/shader.hpp"
#include "../camera.hpp"
#include "../core/texture.hpp"
#include "glm/glm.hpp"
#include "mesh.hpp"
#include <vector>
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>

namespace graphics {

    class Texture2D;

    class MeshRenderer {
    public:
        MeshRenderer() = default;

        unsigned int draw(const Mesh &_mesh, Texture2D::Handle _texture, Texture2D::Handle _phongData, const glm::mat4 &_transform);
        
        void setTransform(const unsigned int &meshID, const glm::mat4 &_transform);
        
        void transform(const unsigned int &meshID, const glm::mat4 &_transform);

        void present(const unsigned int &programID);

        void clear();

    private:
        unsigned int currentProgramID = -1;
        GLint glsl_object_to_world_matrix = 0;

        struct MeshRenderData {
            Mesh meshData;
            Texture2D::Handle textureData = nullptr;
            Texture2D::Handle phongData = nullptr;
            glm::mat4 transform;
        };

        std::vector<MeshRenderData *> meshBuffer = {};

    };
}