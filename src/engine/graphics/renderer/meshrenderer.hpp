#pragma once

#include "../core/texture.hpp"
#include "glm/glm.hpp"
#include "mesh.hpp"
#include <vector>
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>
#include "engine/components/mesh.h"
#include "engine/components/transform.h"
#include <engine/graphics/resources.hpp>

namespace graphics {

    class Texture2D;

    // TODO this entire thing is really not great, please fix.
    class MeshRenderer {
    public:
        MeshRenderer() = default;

        [[nodiscard]] unsigned int requestNewMesh();

        void draw(components::Mesh &_mesh, components::Transform &_transform);

        unsigned int draw(Mesh &_mesh, Texture2D::Handle _texture, Texture2D::Handle _phongData, const glm::mat4 &_transform);

        void setTransform(unsigned int meshID, const glm::mat4 &_transform);

        void transform(unsigned int meshID, const glm::mat4 &_transform);

        void present(unsigned int programID);

        void clear();

    private:
        unsigned int currentProgramID = -1;
        GLint glsl_object_to_world_matrix = 0;

        struct MeshRenderData {
            MeshRenderData(Mesh *meshData, const Texture2D *textureData, const Texture2D *phongData, const Texture2D *heightData, const glm::mat4 &transform)
                    : meshData(meshData),
                      textureData(textureData == nullptr
                                  ? graphics::Texture2DManager::get("textures/fallback/texture.png", *graphics::Sampler::getLinearMirroredSampler())
                                  : textureData),
                      phongData(phongData == nullptr
                                ? graphics::Texture2DManager::get("textures/fallback/phong.png", *graphics::Sampler::getLinearMirroredSampler())
                                : phongData),
                      heightData(heightData == nullptr
                                 ? graphics::Texture2DManager::get("textures/fallback/heightmap.png", *graphics::Sampler::getLinearMirroredSampler())
                                 : heightData),
                      transform(transform) {}

            Mesh *meshData = nullptr;
            Texture2D::Handle textureData = nullptr;
            Texture2D::Handle phongData = nullptr;
            Texture2D::Handle heightData = nullptr;
            glm::mat4 transform = glm::identity<glm::mat4>();

            ~MeshRenderData() {
                delete meshData;
            }
        };

        std::vector<MeshRenderData *> meshBuffer = {};
    };
}