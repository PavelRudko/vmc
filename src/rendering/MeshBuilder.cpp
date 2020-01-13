#include "MeshBuilder.h"

namespace vmc
{
    const glm::vec3 CubeVertices[6][4] =
    {
        { {-1, 1, 1}, {1, 1, 1}, {1, 1, -1}, {-1, 1, -1} }, //top
        { {-1, -1, -1}, {1, -1, -1}, {1, -1, 1}, {-1, -1, 1} }, //bottom
        { {-1, -1, 1}, {1, -1, 1}, {1, 1, 1}, {-1, 1, 1} }, //front
        { {1, -1, -1}, {-1, -1, -1}, {-1, 1, -1}, {1, 1, -1} }, //back
        { {1, -1, 1}, {1, -1, -1}, {1, 1, -1}, {1, 1, 1} }, //right
        { {-1, -1, -1}, {-1, -1, 1}, {-1, 1, 1}, {-1, 1, -1} } //left
    };

    MeshBuilder::MeshBuilder(const VulkanDevice& device, const std::vector<Block>& blockDescriptions) :
        blockDescriptions(blockDescriptions),
        device(device)
    {
    }

    std::unique_ptr<Mesh> MeshBuilder::buildChunkMesh(StagingManager& stagingManager, const Chunk& chunk) const
    {
        std::vector<BlockVertex> vertices;
        std::vector<uint32_t> indices;

        for (uint32_t y = 0; y < ChunkHeight; y++) {
            for (uint32_t z = 0; z < ChunkLength; z++) {
                for (uint32_t x = 0; x < ChunkWidth; x++) {
                    auto blockId = chunk.getBlock(x, y, z);
                    if (blockId == 0) {
                        continue;
                    }
                    addCube(vertices, indices, blockDescriptions[blockId].uvs, {x, y, z});
                }
            }
        }

        return createMesh(stagingManager, vertices, indices);
    }

    std::unique_ptr<Mesh> MeshBuilder::buildBlockMesh(StagingManager& stagingManager, BlockId blockId) const
    {
        std::vector<BlockVertex> vertices;
        std::vector<uint32_t> indices;

        addCube(vertices, indices, blockDescriptions[blockId].uvs);

        return createMesh(stagingManager, vertices, indices);
    }

    void MeshBuilder::addCube(std::vector<BlockVertex>& vertices, std::vector<uint32_t>& indices, const std::vector<glm::vec2>& uvs, const glm::vec3& center) const
    {
        static float halfSize = 0.5f;

        uint32_t uvIndex = 0;
        for (uint32_t face = 0; face < 6; face++) {
            uint32_t baseIndex = vertices.size();

            for (int i = 0; i < 4; i++) {
                vertices.push_back({});
                vertices[baseIndex + i].position = glm::vec4(CubeVertices[face][i] * halfSize + center, 1.0f);
                vertices[baseIndex + i].uv = uvs[uvIndex++];
            }

            indices.push_back(baseIndex + 0);
            indices.push_back(baseIndex + 1);
            indices.push_back(baseIndex + 2);
            indices.push_back(baseIndex + 0);
            indices.push_back(baseIndex + 2);
            indices.push_back(baseIndex + 3);
        }
    }

    std::unique_ptr<Mesh> MeshBuilder::createMesh(StagingManager& stagingManager, const std::vector<BlockVertex>& vertices, const std::vector<uint32_t>& indices) const
    {
        VulkanBuffer vertexBuffer(device, vertices.size() * sizeof(BlockVertex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
        VulkanBuffer indexBuffer(device, indices.size() * sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

        stagingManager.copyToBuffer(vertices.data(), vertexBuffer, 0, vertexBuffer.getSize());
        stagingManager.copyToBuffer(indices.data(), indexBuffer, 0, indexBuffer.getSize());

        return std::make_unique<Mesh>(std::move(vertexBuffer), std::move(indexBuffer), indices.size());
    }
}