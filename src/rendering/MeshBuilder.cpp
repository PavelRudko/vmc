#include "MeshBuilder.h"

namespace vmc
{
    MeshBuilder::MeshBuilder(const VulkanDevice& device) :
        device(device)
    {
    }

    std::unique_ptr<Mesh> MeshBuilder::buildCubeMesh(StagingManager& stagingManager)
    {
        float t = 0.03125f;
        float s = 0.5f;

        std::vector<BlockVertex> vertices = {
            {{-s, -s, -s, 1}, {0, t}},
            {{s, -s, -s, 1},  {t, t}},
            {{s, s, -s, 1},   {t, 0}},
            {{-s, s, -s, 1},  {0, 0}},

            {{-s, -s, s, 1}, {0, t}},
            {{s, -s, s, 1},  {t, t}},
            {{s, s, s, 1},   {t, 0}},
            {{-s, s, s, 1},  {0, 0}}
        };

        std::vector<uint32_t> indices = {
            0, 1, 2, 2, 3, 0,
            4, 5, 6, 6, 7, 4
        };

        VulkanBuffer vertexBuffer(device, vertices.size() * sizeof(BlockVertex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
        VulkanBuffer indexBuffer(device, indices.size() * sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

        stagingManager.copyToBuffer(vertices.data(), vertexBuffer, 0, vertexBuffer.getSize());
        stagingManager.copyToBuffer(indices.data(), indexBuffer, 0, indexBuffer.getSize());

        return std::make_unique<Mesh>(std::move(vertexBuffer), std::move(indexBuffer), indices.size());
    }
}