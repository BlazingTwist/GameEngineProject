#include "meshrenderer.hpp"

namespace graphics {

    MeshRenderer::MeshRenderData::MeshRenderData(Mesh *_meshData,
                                                 const Texture2D *_textureData, const Texture2D *_phongData, const Texture2D *_normalData,
                                                 const Texture2D *_heightData,
                                                 const glm::mat4 &_transform)
            : meshData(_meshData), transform(_transform) {
        setTextureData(_textureData);
        setPhongData(_phongData);
        setNormalData(_normalData);
        setHeightData(_heightData);
    }

    void MeshRenderer::MeshRenderData::setTextureData(const Texture2D *_textureData) {
        MeshRenderData::textureData = _textureData == nullptr
                                      ? graphics::Texture2DManager::get("textures/fallback/texture.png",
                                                                        *graphics::Sampler::getLinearMirroredSampler())
                                      : _textureData;
    }

    void MeshRenderer::MeshRenderData::setPhongData(const Texture2D *_phongData) {
        MeshRenderData::phongData = _phongData == nullptr
                                    ? graphics::Texture2DManager::get("textures/fallback/phong.png",
                                                                      *graphics::Sampler::getLinearMirroredSampler())
                                    : _phongData;
    }

    void MeshRenderer::MeshRenderData::setNormalData(const Texture2D *_normalData) {
        MeshRenderData::normalData = _normalData == nullptr
                                     ? graphics::Texture2DManager::get("textures/fallback/normal.png",
                                                                       *graphics::Sampler::getLinearMirroredSampler())
                                     : _normalData;
    }

    void MeshRenderer::MeshRenderData::setHeightData(const Texture2D *_heightData) {
        MeshRenderData::heightData = _heightData == nullptr
                                     ? graphics::Texture2DManager::get("textures/fallback/heightmap.png",
                                                                       *graphics::Sampler::getLinearMirroredSampler())
                                     : _heightData;
    }

    void MeshRenderer::registerMesh(const entity::EntityReference *entity) {
        entity::EntityRegistry &registry = entity::EntityRegistry::getInstance();

        components::Mesh mesh = registry.getComponentData<components::Mesh>(entity).value();
        components::Transform transform = registry.getComponentData<components::Transform>(entity).value();

        if (mesh._rendererID >= 0) {
            spdlog::error("mesh was already registered! Ignoring registerMesh call.");
            return;
        }

        mesh._rendererID = static_cast<int>(registeredMeshCount);
        registeredMeshCount++;
        activeMeshEntities.push_back(entity);

        auto *data = new MeshRenderData(new Mesh(mesh.getMeshData()), mesh.getTextureData(), mesh.getPhongData(), mesh.getNormalData(),
                                        mesh.getHeightData(), transform.getTransformMatrix());
        data->isEnabled = mesh.getIsEnabled();
        meshBuffer.push_back(data);

        mesh.meshChangesHandled();
        mesh.textureChangesHandled();
        mesh.phongChangesHandled();
        mesh.normalChangesHandled();
        mesh.heightChangesHandled();
        transform.onChangesHandled();

        registry.addOrSetComponent(entity, mesh);
        registry.addOrSetComponent(entity, transform);
    }

    void MeshRenderer::update() {
        entity::EntityRegistry &registry = entity::EntityRegistry::getInstance();

        for (const entity::EntityReference *entity: activeMeshEntities) {
            components::Mesh mesh = registry.getComponentData<components::Mesh>(entity).value();
            components::Transform transform = registry.getComponentData<components::Transform>(entity).value();

            MeshRenderData *data = meshBuffer[mesh._rendererID];
            data->isEnabled = mesh.getIsEnabled();
            if (mesh.hasAnyChanges()) {
                if (mesh.meshHasChanged()) {
                    delete data->meshData;
                    data->meshData = new Mesh(mesh.getMeshData());
                    mesh.meshChangesHandled();
                }
                if (mesh.textureHasChanged()) {
                    data->setTextureData(mesh.getTextureData());
                    mesh.textureChangesHandled();
                }
                if (mesh.phongHasChanged()) {
                    data->setPhongData(mesh.getPhongData());
                    mesh.phongChangesHandled();
                }
                if (mesh.normalHasChanged()) {
                    data->setNormalData(mesh.getNormalData());
                    mesh.normalChangesHandled();
                }
                if (mesh.heightHasChanged()) {
                    data->setHeightData(mesh.getHeightData());
                    mesh.heightChangesHandled();
                }
                registry.addOrSetComponent(entity, mesh);
            }
            if (transform.hasTransformChanged()) {
                data->transform = transform.getTransformMatrix();
                transform.onChangesHandled();
                registry.addOrSetComponent(entity, transform);
            }
        }
    }

    void MeshRenderer::removeMesh(const entity::EntityReference *meshEntity) {
        removeMesh(entity::EntityRegistry::getInstance().getComponentData<components::Mesh>(meshEntity).value());
    }

    void MeshRenderer::removeMesh(const components::Mesh &mesh) {
        if (registeredMeshCount == 0) {
            spdlog::error("tried to remove mesh, but MeshRenderer is empty!");
            return;
        }

        registeredMeshCount--;
        delete meshBuffer[mesh._rendererID];
        if (mesh._rendererID != registeredMeshCount) {
            meshBuffer[mesh._rendererID] = meshBuffer.back();
            const entity::EntityReference *moveEntity = activeMeshEntities.back();
            activeMeshEntities[mesh._rendererID] = moveEntity;

            components::Mesh moveMesh = entity::EntityRegistry::getInstance().getComponentData<components::Mesh>(moveEntity).value();
            moveMesh._rendererID = mesh._rendererID;
            entity::EntityRegistry::getInstance().addOrSetComponent(moveEntity, moveMesh);
        }
        meshBuffer.pop_back();
        activeMeshEntities.pop_back();
    }

    void MeshRenderer::present(const unsigned int programID) {
        if (currentProgramID != programID) {
            currentProgramID = programID;
            glsl_object_to_world_matrix = glGetUniformLocation(programID, "object_to_world_matrix");
        }

        for (const MeshRenderData *meshRenderData: meshBuffer) {
            if (meshRenderData == nullptr || !meshRenderData->isEnabled) {
                continue;
            }
            meshRenderData->textureData->bind(0);
            meshRenderData->phongData->bind(1);
            meshRenderData->normalData->bind(2);
            meshRenderData->heightData->bind(3);
            glUniformMatrix4fv(glsl_object_to_world_matrix, 1, false, glm::value_ptr(meshRenderData->transform));
            meshRenderData->meshData->getGeometryBuffer()->draw();
        }
    }

    void MeshRenderer::clear() {
        for (auto *item: meshBuffer) {
            delete item;
        }
        meshBuffer.clear();

        activeMeshEntities.clear();

        registeredMeshCount = 0;
    }
}
