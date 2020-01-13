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

    std::vector<glm::vec2> calculateNormalizedUvs(const std::vector<glm::vec2>& ranges, uint32_t textureWidth, uint32_t textureHeight)
    {
        std::vector<glm::vec2> uvs;
        for (uint32_t i = 0; i < ranges.size(); i += 2) {
            auto uvMin = ranges[i];
            auto uvMax = ranges[i + 1];

            uvMin.x /= textureWidth;
            uvMin.y /= textureHeight;

            uvMax.x /= textureWidth;
            uvMax.y /= textureHeight;

            uvs.push_back({ uvMin.x, uvMax.y });
            uvs.push_back({ uvMax.x, uvMax.y });
            uvs.push_back({ uvMax.x, uvMin.y });
            uvs.push_back({ uvMin.x, uvMin.y });
        }

        uint32_t baseIndex = uvs.size() - 4;
        uint32_t facesLeft = 6 - uvs.size() / 4;
        for (uint32_t i = 0; i < facesLeft; i++) {
            uvs.push_back(uvs[baseIndex + 0]);
            uvs.push_back(uvs[baseIndex + 1]);
            uvs.push_back(uvs[baseIndex + 2]);
            uvs.push_back(uvs[baseIndex + 3]);
        }

        return uvs;
    }

    MeshBuilder::MeshBuilder(const VulkanDevice& device) :
        device(device)
    {
        blocks.resize(256);

        blocks[1].uvs = calculateNormalizedUvs(
        {
            { 16, 0 }, { 32, 16 },
            { 32, 0 }, { 48, 16 },
            { 0, 0 }, { 16, 16 }
        }, 512, 512);
    }

    std::unique_ptr<Mesh> MeshBuilder::buildBlockMesh(StagingManager& stagingManager, BlockId blockId) const
    {
        std::vector<BlockVertex> vertices;
        std::vector<uint32_t> indices;

        addCube(vertices, indices, blocks[blockId].uvs);

        VulkanBuffer vertexBuffer(device, vertices.size() * sizeof(BlockVertex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
        VulkanBuffer indexBuffer(device, indices.size() * sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

        stagingManager.copyToBuffer(vertices.data(), vertexBuffer, 0, vertexBuffer.getSize());
        stagingManager.copyToBuffer(indices.data(), indexBuffer, 0, indexBuffer.getSize());

        return std::make_unique<Mesh>(std::move(vertexBuffer), std::move(indexBuffer), indices.size());
    }

    void MeshBuilder::addCube(std::vector<BlockVertex>& vertices, std::vector<uint32_t>& indices, const std::vector<glm::vec2>& uvs) const
    {
        static float halfSize = 0.5f;

        uint32_t uvIndex = 0;
        for (uint32_t face = 0; face < 6; face++) {
            uint32_t baseIndex = vertices.size();

            for (int i = 0; i < 4; i++) {
                vertices.push_back({});
                vertices[baseIndex + i].position = glm::vec4(CubeVertices[face][i] * halfSize, 1.0f);
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
}