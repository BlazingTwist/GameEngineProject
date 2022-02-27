#ifndef ACAENGINE_MESH_H
#define ACAENGINE_MESH_H

#include "engine/graphics/renderer/mesh.hpp"
#include "engine/graphics/core/texture.hpp"

namespace components {

    /**
     * Describes a collection of triangles (i.e. Mesh) and its textures.
     * 
     * Changes to this mesh should be made during the update tick.
     * Changes are then managed and cached during the draw tick.
     */
    class Mesh {
        static constexpr int meshStateIndex = 0;
        static constexpr int textureStateIndex = 1;
        static constexpr int phongStateIndex = 2;

    public:
        Mesh() : _rendererID(0) {};

        Mesh(const unsigned int rendererId,
             const utils::MeshData *meshData,
             const graphics::Texture2D *textureData = nullptr,
             const graphics::Texture2D *phongData = nullptr) :
                _rendererID(rendererId),
                _meshData(meshData),
                _textureData(textureData),
                _phongData(phongData) {

            _stateChanges |= 0b1 << meshStateIndex;
            if (textureData != nullptr) {
                _stateChanges |= 0b1 << textureStateIndex;
            }
            if (phongData != nullptr) {
                _stateChanges |= 0b1 << phongStateIndex;
            }
        }

        [[nodiscard]] unsigned int getRendererId() const {
            return _rendererID;
        }

        [[nodiscard]] utils::MeshData::Handle getMeshData() const {
            return _meshData;
        }

        void setMeshData(utils::MeshData::Handle meshData) {
            Mesh::_meshData = meshData;
            _stateChanges |= 0b1 << meshStateIndex;
        }

        [[nodiscard]] const graphics::Texture2D *getTextureData() const {
            return _textureData;
        }

        void setTextureData(const graphics::Texture2D *textureData) {
            Mesh::_textureData = textureData;
            _stateChanges |= 0b1 << textureStateIndex;
        }

        [[nodiscard]] const graphics::Texture2D *getPhongData() const {
            return _phongData;
        }

        void setPhongData(const graphics::Texture2D *phongData) {
            Mesh::_phongData = phongData;
            _stateChanges |= 0b1 << phongStateIndex;
        }

        [[nodiscard]] bool meshHasChanged() const {
            return (_stateChanges & (0b1 << meshStateIndex)) != 0;
        }

        void meshChangesHandled() {
            _stateChanges &= ~(0b1 << meshStateIndex);
        }

        [[nodiscard]] bool textureHasChanged() const {
            return (_stateChanges & (0b1 << textureStateIndex)) != 0;
        }

        void textureChangesHandled() {
            _stateChanges &= ~(0b1 << textureStateIndex);
        }

        [[nodiscard]] bool phongHasChanged() const {
            return (_stateChanges & (0b1 << phongStateIndex)) != 0;
        }

        void phongChangesHandled() {
            _stateChanges &= ~(0b1 << phongStateIndex);
        }

        [[nodiscard]] bool hasAnyChanges() const {
            return _stateChanges != 0;
        }

    private:
        unsigned int _rendererID;

        utils::MeshData::Handle _meshData = nullptr;
        graphics::Texture2D::Handle _textureData = nullptr;
        graphics::Texture2D::Handle _phongData = nullptr;

        int _stateChanges = 0;
    };
}

#endif //ACAENGINE_MESH_H
