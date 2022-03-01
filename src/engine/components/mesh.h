#ifndef ACAENGINE_MESH_H
#define ACAENGINE_MESH_H

#include "engine/graphics/renderer/mesh.hpp"
#include "engine/graphics/core/texture.hpp"

namespace graphics {
    class MeshRenderer;
}

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
        static constexpr int normalStateIndex = 3;
        static constexpr int heightStateIndex = 4;

    public:
        Mesh() = default;

        explicit Mesh(const utils::MeshData *meshData,
                      const graphics::Texture2D *textureData = nullptr,
                      const graphics::Texture2D *phongData = nullptr,
                      const graphics::Texture2D *normalData = nullptr,
                      const graphics::Texture2D *heightData = nullptr) :
                _meshData(meshData),
                _textureData(textureData),
                _phongData(phongData),
                _normalData(normalData),
                _heightData(heightData) {

            _stateChanges |= 0b1 << meshStateIndex;
            if (textureData != nullptr) {
                _stateChanges |= 0b1 << textureStateIndex;
            }
            if (phongData != nullptr) {
                _stateChanges |= 0b1 << phongStateIndex;
            }
            if (normalData != nullptr) {
                _stateChanges |= 0b1 << normalStateIndex;
            }
            if (heightData != nullptr) {
                _stateChanges |= 0b1 << heightStateIndex;
            }
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

        [[nodiscard]] const graphics::Texture2D *getNormalData() const {
            return _normalData;
        }

        void setNormalData(const graphics::Texture2D *normalData) {
            Mesh::_normalData = normalData;
            _stateChanges |= 0b1 << normalStateIndex;
        }

        [[nodiscard]] const graphics::Texture2D *getHeightData() const {
            return _heightData;
        }

        void setHeightData(const graphics::Texture2D *heightData) {
            Mesh::_heightData = heightData;
            _stateChanges |= 0b1 << heightStateIndex;
        }

        [[nodiscard]] bool getIsEnabled() const {
            return _isEnabled;
        }

        // This should only be used to disable meshes temporarily, not to permanently delete it.
        void setEnabled(bool isEnabled) {
            Mesh::_isEnabled = isEnabled;
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

        [[nodiscard]] bool normalHasChanged() const {
            return (_stateChanges & (0b1 << normalStateIndex)) != 0;
        }

        void normalChangesHandled() {
            _stateChanges &= ~(0b1 << normalStateIndex);
        }

        [[nodiscard]] bool heightHasChanged() const {
            return (_stateChanges & (0b1 << heightStateIndex)) != 0;
        }

        void heightChangesHandled() {
            _stateChanges &= ~(0b1 << heightStateIndex);
        }

        [[nodiscard]] bool hasAnyChanges() const {
            return _stateChanges != 0;
        }

    protected:
        int _rendererID = -1;

        friend class graphics::MeshRenderer;

    private:
        utils::MeshData::Handle _meshData = nullptr;
        graphics::Texture2D::Handle _textureData = nullptr;
        graphics::Texture2D::Handle _phongData = nullptr;
        graphics::Texture2D::Handle _normalData = nullptr;
        graphics::Texture2D::Handle _heightData = nullptr;
        bool _isEnabled = true;

        int _stateChanges = 0;
    };
}

#endif //ACAENGINE_MESH_H
