#include "MeshBuilder.h"

namespace vmc
{
    const glm::ivec3 AdjascentDirections[6]
    {
        {0, -1, 0},
        {0, 1, 0},
        {0, 0, 1},
        {0, 0, -1},
        {-1, 0, 0},
        {1, 0, 0}
    };

    const Faces AdjascentFaces[6]
    {
        Faces::Top,
        Faces::Bottom,
        Faces::Front,
        Faces::Back,
        Faces::Right,
        Faces::Left
    };

    const glm::vec3 CubeVertices[6][4] =
    {
        { {-1, 1, 1}, {1, 1, 1}, {1, 1, -1}, {-1, 1, -1} }, //top
        { {-1, -1, -1}, {1, -1, -1}, {1, -1, 1}, {-1, -1, 1} }, //bottom
        { {-1, -1, 1}, {1, -1, 1}, {1, 1, 1}, {-1, 1, 1} }, //front
        { {1, -1, -1}, {-1, -1, -1}, {-1, 1, -1}, {1, 1, -1} }, //back
        { {1, -1, 1}, {1, -1, -1}, {1, 1, -1}, {1, 1, 1} }, //right
        { {-1, -1, -1}, {-1, -1, 1}, {-1, 1, 1}, {-1, 1, -1} } //left
    };

    const glm::vec3 CrossVertices[2][4] =
    {
        { {-0.5, 0, -0.5}, {0.5, 0, 0.5}, {0.5, 1, 0.5}, {-0.5, 1, -0.5} },
        { {-0.5, 0, 0.5}, {0.5, 0, -0.5}, {0.5, 1, -0.5}, {-0.5, 1, 0.5} }
    };

    void addCube(std::vector<BlockVertex>& vertices, std::vector<uint32_t>& indices, const Block& description, const glm::vec3& center, uint8_t visibleFaces)
    {
        static float halfSize = 0.5f;

        uint32_t uvIndex = 0;
        for (uint32_t face = 0; face < 6; face++) {
            if ((visibleFaces & AdjascentFaces[face]) != 0) {
                uint32_t baseIndex = vertices.size();

                for (int i = 0; i < 4; i++) {
                    vertices.push_back({});
                    vertices[baseIndex + i].position = glm::vec4(CubeVertices[face][i] * halfSize + center, 1.0f);
                    vertices[baseIndex + i].uv = description.uvs[uvIndex + i];
                }

                indices.push_back(baseIndex + 0);
                indices.push_back(baseIndex + 1);
                indices.push_back(baseIndex + 2);
                indices.push_back(baseIndex + 0);
                indices.push_back(baseIndex + 2);
                indices.push_back(baseIndex + 3);
            }

            uvIndex += 4;
        }
    }

    void addCross(std::vector<BlockVertex>& vertices, std::vector<uint32_t>& indices, const Block& description, const glm::vec3& center, uint8_t visibleFaces)
    {
        auto origin = center - glm::vec3(0, 0.5, 0);
        glm::vec3 scale(description.width, description.height, description.width);
        uint32_t uvIndex = 0;
        for (uint32_t face = 0; face < 2; face++) {
            uint32_t baseIndex = vertices.size();

            for (int i = 0; i < 4; i++) {
                vertices.push_back({});
                vertices[baseIndex + i].position = glm::vec4(CrossVertices[face][i] * scale + origin, 1.0f);
                vertices[baseIndex + i].uv = description.uvs[uvIndex + i];
            }

            indices.push_back(baseIndex + 0);
            indices.push_back(baseIndex + 1);
            indices.push_back(baseIndex + 2);
            indices.push_back(baseIndex + 0);
            indices.push_back(baseIndex + 2);
            indices.push_back(baseIndex + 3);

            uvIndex += 4;
        }
    }

    MeshBuilder::MeshBuilder(const VulkanDevice& device, const std::vector<Block>& blockDescriptions) :
        blockDescriptions(blockDescriptions),
        device(device)
    {
    }

    Mesh MeshBuilder::buildChunkMesh(StagingManager& stagingManager, const World& world, const Chunk& chunk, const glm::ivec2& chunkCoordinate) const
    {
        static std::vector<uint8_t> visibleChunkFaces(ChunkHeight * ChunkWidth * ChunkLength, 0);
        memset(visibleChunkFaces.data(), 0, visibleChunkFaces.size());

        std::vector<BlockVertex> vertices;
        std::vector<uint32_t> indices;

        for (uint32_t y = 0; y < ChunkHeight; y++) {
            for (uint32_t z = 0; z < ChunkLength; z++) {
                for (uint32_t x = 0; x < ChunkWidth; x++) {
                    auto blockId = chunk.getBlock(x, y, z);
                    const auto& description = blockDescriptions[blockId];
                    if (!description.isOpaque) {
                        addAdjascent({ x, y, z }, chunk, visibleChunkFaces);
                        if (blockId != AirBlockId) {
                            addTransparentBlock({ x, y, z }, chunk, visibleChunkFaces);
                        }
                    }
                    else if (x == 0 || x == ChunkWidth - 1 || z == 0 || z == ChunkLength - 1) {
                        addBoundaryBlock({ x, y, z }, world, chunk, visibleChunkFaces, chunkCoordinate);
                    }
                }
            }
        }

        for (uint32_t y = 0; y < ChunkHeight; y++) {
            for (uint32_t z = 0; z < ChunkLength; z++) {
                for (uint32_t x = 0; x < ChunkWidth; x++) {
                    size_t index = getIndexInChunk(x, y, z);
                    auto blockId = chunk.getBlock(x, y, z);
                    const auto& description = blockDescriptions[blockId];
                    uint8_t visibleFaces = visibleChunkFaces[index];
                    if (visibleFaces != Faces::None) {
                        if (description.shape == BlockShape::Cube) {
                            addCube(vertices, indices, description, { x, y, z }, visibleFaces);
                        }
                        else {
                            addCross(vertices, indices, description, { x, y, z }, visibleFaces);
                        }
                    }
                }
            }
        }

        return createMesh(stagingManager, vertices, indices);
    }

    Mesh MeshBuilder::buildBlockMesh(StagingManager& stagingManager, BlockId blockId) const
    {
        std::vector<BlockVertex> vertices;
        std::vector<uint32_t> indices;

        addCube(vertices, indices, blockDescriptions[blockId], { 0, 0, 0 }, Faces::All);

        return createMesh(stagingManager, vertices, indices);
    }

    void MeshBuilder::addAdjascent(const glm::ivec3& position, const Chunk& chunk, std::vector<uint8_t>& chunkFaces) const
    {
        for (uint32_t i = 0; i < 6; i++) {
            auto coord = position + AdjascentDirections[i];
            if (isOutOfChunkBounds(coord)) {
                continue;
            }

            size_t index = getIndexInChunk(coord.x, coord.y, coord.z);
            if (blockDescriptions[chunk.getData()[index]].isOpaque) {
                chunkFaces[index] |= AdjascentFaces[i];
            }
        }
    }

    void MeshBuilder::addBoundaryBlock(const glm::ivec3& position, const World& world, const Chunk& chunk, std::vector<uint8_t>& chunkFaces, const glm::ivec2& chunkCoordinate) const
    {
        size_t index = getIndexInChunk(position.x, position.y, position.z);
        auto worldPosition = position + glm::ivec3(chunkCoordinate[0] * ChunkWidth, 0, chunkCoordinate[1] * ChunkLength);

        if (position.x == 0) {
            auto adjascentCoord = position;
            adjascentCoord.x = ChunkWidth - 1;
            const auto& adjascentChunk = world.getChunk(worldPosition + glm::ivec3(-1, 0, 0));
            if (adjascentChunk == nullptr || !isOpaque(adjascentChunk->getBlock(adjascentCoord.x, adjascentCoord.y, adjascentCoord.z))) {
                chunkFaces[index] |= Left;
            }
        }
        else if (position.x == ChunkWidth - 1) {
            auto adjascentCoord = position;
            adjascentCoord.x = 0;
            const auto& adjascentChunk = world.getChunk(worldPosition + glm::ivec3(1, 0, 0));
            if (adjascentChunk == nullptr || !isOpaque(adjascentChunk->getBlock(adjascentCoord.x, adjascentCoord.y, adjascentCoord.z))) {
                chunkFaces[index] |= Right;
            }
        }

        if (position.z == 0) {
            auto adjascentCoord = position;
            adjascentCoord.z = ChunkLength - 1;
            const auto& adjascentChunk = world.getChunk(worldPosition + glm::ivec3(0, 0, -1));
            if (adjascentChunk == nullptr || !isOpaque(adjascentChunk->getBlock(adjascentCoord.x, adjascentCoord.y, adjascentCoord.z))) {
                chunkFaces[index] |= Back;
            }
        }
        else if (position.z == ChunkLength - 1) {
            auto adjascentCoord = position;
            adjascentCoord.z = 0;
            const auto& adjascentChunk = world.getChunk(worldPosition + glm::ivec3(0, 0, 1));
            if (adjascentChunk == nullptr || !isOpaque(adjascentChunk->getBlock(adjascentCoord.x, adjascentCoord.y, adjascentCoord.z))) {
                chunkFaces[index] |= Front;
            }
        }
    }

    void MeshBuilder::addTransparentBlock(const glm::ivec3& position, const Chunk& chunk, std::vector<uint8_t>& chunkFaces) const
    {
        size_t index = getIndexInChunk(position.x, position.y, position.z);
        uint8_t faces = chunkFaces[index];

        for (uint32_t i = 0; i < 6; i++) {
            auto coord = position + AdjascentDirections[i];
            if (isOutOfChunkBounds(coord) || !isOpaque(chunk.getBlock(coord.x, coord.y, coord.z))) {
                faces |= AdjascentFaces[i];
            }
        }

        chunkFaces[index] = faces;
    }

    Mesh MeshBuilder::createMesh(StagingManager& stagingManager, const std::vector<BlockVertex>& vertices, const std::vector<uint32_t>& indices) const
    {
        VulkanBuffer vertexBuffer(device, vertices.size() * sizeof(BlockVertex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
        VulkanBuffer indexBuffer(device, indices.size() * sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

        stagingManager.copyToBuffer(vertices.data(), vertexBuffer, 0, vertexBuffer.getSize());
        stagingManager.copyToBuffer(indices.data(), indexBuffer, 0, indexBuffer.getSize());

        return Mesh(std::move(vertexBuffer), std::move(indexBuffer), indices.size());
    }

    bool MeshBuilder::isOpaque(BlockId id) const
    {
        return blockDescriptions[id].isOpaque;
    }
}