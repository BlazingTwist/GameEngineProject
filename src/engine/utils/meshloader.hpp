#pragma once

#include "resourcemanager.hpp"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <array>
#include <optional>
#include <vector>

namespace utils
{

	/**
	 * @brief Collection of raw mesh data.
	 */
	struct MeshData
	{
		/**
		 * @brief Collection of vertices describing one face.
		 * contains 3 ordered vertices
		 */
		struct FaceData
		{
			/**
			 * @brief Collection of ids describing one vertex.
			 */
			struct VertexIndices
			{
				int positionIdx;
				///< id to fetch the position from MeshData::positions
				std::optional<int> textureCoordinateIdx;
				///< id to fetch the texture Coordinates from MeshData::textureCoordinates
				///< may be not set for a vertex
				std::optional<int> normalIdx;
				///< id to fetch the normal from MeshData::normals
				///< may be not set for a vertex

                bool operator==(const MeshData::FaceData::VertexIndices &rhs) const {
                    return positionIdx == rhs.positionIdx
                           && textureCoordinateIdx == rhs.textureCoordinateIdx
                           && normalIdx == rhs.normalIdx;
                }
			};
			std::array<VertexIndices, 3> indices;
		};

		using Handle = const MeshData*;


		/**
		 * @brief load mesh data from an file
		 * @param _fileName path to file
		 */
		static Handle load( const char* _fileName );

		static void unload( Handle _meshData );

		std::vector<glm::vec3> positions;
		std::vector<glm::vec2> textureCoordinates;
		std::vector<glm::vec3> normals;
		std::vector<FaceData>  faces;
	};

	using MeshLoader = utils::ResourceManager<MeshData>;

} // end utils
