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
#include <engine/entity/entityregistry.h>

namespace graphics {

    class Texture2D;

    class MeshRenderer {

    private:
        struct MeshRenderData {
            MeshRenderData(Mesh *_meshData,
                           const Texture2D *_textureData, const Texture2D *_phongData, const Texture2D *_normalData, const Texture2D *_heightData,
                           const glm::mat4 &_transform);

            void setTextureData(const Texture2D *_textureData);

            void setPhongData(const Texture2D *_phongData);

            void setNormalData(const Texture2D *_normalData);

            void setHeightData(const Texture2D *_heightData);

            Mesh *meshData = nullptr;
            Texture2D::Handle textureData = nullptr;
            Texture2D::Handle phongData = nullptr;
            Texture2D::Handle normalData = nullptr;
            Texture2D::Handle heightData = nullptr;
            glm::mat4 transform = glm::identity<glm::mat4>();
            bool isEnabled = true;

            ~MeshRenderData() {
                delete meshData;
            }
        };

    public:
        MeshRenderer() = default;

        void registerMesh(const entity::EntityReference *entity);

        void update();

        void removeMesh(const entity::EntityReference *meshEntity);
        
        void removeMesh(const components::Mesh &mesh);

        void present(unsigned int programID);

        void clear();

    private:
        unsigned int currentProgramID = -1;
        GLint glsl_object_to_world_matrix = 0;

        unsigned int registeredMeshCount = 0;
        std::vector<const entity::EntityReference *> activeMeshEntities = {};
        std::vector<MeshRenderData *> meshBuffer = {};

    };
}