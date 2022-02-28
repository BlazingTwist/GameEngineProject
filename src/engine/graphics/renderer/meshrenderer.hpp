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
            MeshRenderData(Mesh *_meshData,
                           const Texture2D *_textureData, const Texture2D *_phongData, const Texture2D *_normalData, const Texture2D *_heightData,
                           const glm::mat4 &_transform)
                    : meshData(_meshData), transform(_transform) {
                setTextureData(_textureData);
                setPhongData(_phongData);
                setNormalData(_normalData);
                setHeightData(_heightData);
            }

            void setTextureData(const Texture2D *_textureData) {
                MeshRenderData::textureData = _textureData == nullptr
                                              ? graphics::Texture2DManager::get("textures/fallback/texture.png",
                                                                                *graphics::Sampler::getLinearMirroredSampler())
                                              : _textureData;
            }

            void setPhongData(const Texture2D *_phongData) {
                MeshRenderData::phongData = _phongData == nullptr
                                            ? graphics::Texture2DManager::get("textures/fallback/phong.png",
                                                                              *graphics::Sampler::getLinearMirroredSampler())
                                            : _phongData;
            }

            void setNormalData(const Texture2D *_normalData) {
                MeshRenderData::normalData = _normalData == nullptr
                                            ? graphics::Texture2DManager::get("textures/fallback/normal.png",
                                                                              *graphics::Sampler::getLinearMirroredSampler())
                                            : _normalData;
            }

            void setHeightData(const Texture2D *_heightData) {
                MeshRenderData::heightData = _heightData == nullptr
                                             ? graphics::Texture2DManager::get("textures/fallback/heightmap.png",
                                                                               *graphics::Sampler::getLinearMirroredSampler())
                                             : _heightData;
            }

            Mesh *meshData = nullptr;
            Texture2D::Handle textureData = nullptr;
            Texture2D::Handle phongData = nullptr;
            Texture2D::Handle normalData = nullptr;
            Texture2D::Handle heightData = nullptr;
            glm::mat4 transform = glm::identity<glm::mat4>();

            ~MeshRenderData() {
                delete meshData;
            }
        };

        std::vector<MeshRenderData *> meshBuffer = {};
    };
}