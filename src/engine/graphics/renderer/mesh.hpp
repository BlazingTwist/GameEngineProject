#pragma once

#include <engine/graphics/core/geometrybuffer.hpp>
#include "../../utils/meshloader.hpp"

namespace graphics {

    struct VertexData {
        glm::vec3 positionData;
        glm::vec2 uvData;
        glm::vec3 normalData;
    };

    class Mesh {
    public:
        Mesh(const utils::MeshData::Handle &_meshData);

        static constexpr std::array<graphics::VertexAttribute, 3> VERTEX_ATTRIBUTES = {
                graphics::VertexAttribute{
                        graphics::PrimitiveFormat::FLOAT,
                        3,
                        false,
                        false
                },
                graphics::VertexAttribute{
                        graphics::PrimitiveFormat::FLOAT,
                        2,
                        false,
                        false
                },
                graphics::VertexAttribute{
                        graphics::PrimitiveFormat::FLOAT,
                        3,
                        false,
                        false
                }
        };

        [[nodiscard]] const graphics::GeometryBuffer *getGeometryBuffer() const { return currentBuffer; }

        ~Mesh() {
            delete currentBuffer;
        }

    private:
        graphics::GeometryBuffer *currentBuffer = nullptr;

    };
}
