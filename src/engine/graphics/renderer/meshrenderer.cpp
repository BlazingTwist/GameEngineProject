#include "meshrenderer.hpp"

namespace graphics {


    unsigned int MeshRenderer::draw(const Mesh &_mesh, Texture2D::Handle _texture, Texture2D::Handle _phongData, const glm::mat4 &_transform) {
        auto *renderData = new MeshRenderData{
                _mesh,
                _texture,
                _phongData,
                _transform
        };
        meshBuffer.push_back(renderData);
        return meshBuffer.size() - 1;
    }

    void MeshRenderer::setTransform(const unsigned int &meshID, const glm::mat4 &_transform){
        if (meshBuffer.size() <= meshID){
            spdlog::error("Attempted to transform mesh at ID: {} - ID does not exist.", meshID);
            return;
        }
        
        meshBuffer[meshID]->transform = _transform;
    }
    
    void MeshRenderer::transform(const unsigned int &meshID, const glm::mat4 &_transform){
        if (meshBuffer.size() <= meshID){
            spdlog::error("Attempted to transform mesh at ID: {} - ID does not exist.", meshID);
            return;
        }

        meshBuffer[meshID]->transform = _transform * meshBuffer[meshID]->transform;
    }

    void MeshRenderer::present(const unsigned int &programID) {
        if (currentProgramID != programID){
            currentProgramID = programID;
            glsl_object_to_world_matrix = glGetUniformLocation(programID, "object_to_world_matrix");
        }
        
        for (const auto &meshRenderData: meshBuffer){
            meshRenderData->textureData->bind(0);
            meshRenderData->phongData->bind(1);
            glUniformMatrix4fv(glsl_object_to_world_matrix, 1, false, glm::value_ptr(meshRenderData->transform));
            for (const auto &geometryBuffer : meshRenderData->meshData.getGeometryBuffers()){
                geometryBuffer->draw();
            }
        }
    }

    void MeshRenderer::clear() {
        meshBuffer.clear();
    }
}
