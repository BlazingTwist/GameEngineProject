#include "mesh.hpp"

namespace graphics {

    Mesh::Mesh(const utils::MeshData::Handle &_meshData) {
        std::vector<const utils::MeshData::FaceData::VertexIndices *> uniqueVertices = {};

        const int vertexCount = static_cast<int>(_meshData->faces.size()) * 3;
        auto *vertexIndices = new int[vertexCount];
        int vertexIndicesIndex = 0;
        for (const auto &face: _meshData->faces) {
            for (const auto &vertex: face.indices) {
                int uniqueIndex = -1;
                for (unsigned int i = 0; i < uniqueVertices.size(); ++i) {
                    if ((*uniqueVertices[i]) == vertex) {
                        uniqueIndex = static_cast<int>(i);
                        break;
                    }
                }
                if (uniqueIndex < 0) {
                    uniqueIndex = static_cast<int>(uniqueVertices.size());
                    uniqueVertices.push_back(&vertex);
                }
                vertexIndices[vertexIndicesIndex] = uniqueIndex;
                vertexIndicesIndex++;
            }
        }

        const unsigned int uniqueVertexCount = uniqueVertices.size();
        auto *vertexBuffer = new graphics::VertexData[uniqueVertexCount];
        for (unsigned int i = 0; i < uniqueVertexCount; i++) {
            const utils::MeshData::FaceData::VertexIndices *uniqueVertex = uniqueVertices[i];
            graphics::VertexData data = graphics::VertexData();
            data.positionData = _meshData->positions[uniqueVertex->positionIdx];

            std::optional<int> uvIndex = uniqueVertex->textureCoordinateIdx;
            if (uvIndex.has_value()) {
                data.uvData = _meshData->textureCoordinates[uvIndex.value()];
            } else {
                data.uvData = glm::vec2();
            }

            std::optional<int> normalIndex = uniqueVertex->normalIdx;
            if (normalIndex.has_value()) {
                data.normalData = _meshData->normals[normalIndex.value()];
            } else {
                data.normalData = glm::vec3();
            }

            vertexBuffer[i] = data;
        }

        currentBuffer = new graphics::GeometryBuffer(
                graphics::GLPrimitiveType::TRIANGLES,
                VERTEX_ATTRIBUTES.data(),
                VERTEX_ATTRIBUTES.size(),
                sizeof(int),
                vertexCount * sizeof(int)
        );
        currentBuffer->setIndexData(vertexIndices, vertexCount * sizeof(int));
        currentBuffer->setData(vertexBuffer, uniqueVertexCount * sizeof(VertexData));
        delete[] vertexIndices;
        delete[] vertexBuffer;
        uniqueVertices.clear();
    }
}