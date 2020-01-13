#include "Mesh.h"
#include <vk/StagingManager.h>
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <world/Block.h>
#include <world/Chunk.h>

namespace vmc
{
       struct BlockVertex
    {
        glm::vec4 position;
        glm::vec2 uv;
    };

    class MeshBuilder
    {
    public:
        MeshBuilder(const VulkanDevice& device, const std::vector<Block>& blockDescriptions);

        MeshBuilder(const MeshBuilder&) = delete;

        MeshBuilder(Mesh&&) = delete;

        MeshBuilder& operator=(const MeshBuilder&) = delete;

        MeshBuilder& operator=(MeshBuilder&&) = delete;

        std::unique_ptr<Mesh> buildChunkMesh(StagingManager& stagingManager, const Chunk& chunk) const;

        std::unique_ptr<Mesh> buildBlockMesh(StagingManager& stagingManager, BlockId blockId) const;

    private:
        const VulkanDevice& device;

        const std::vector<Block>& blockDescriptions;

        void addCube(std::vector<BlockVertex>& vertices, std::vector<uint32_t>& indices, const std::vector<glm::vec2>& uvs, const glm::vec3& center = {0, 0, 0}) const;

        std::unique_ptr<Mesh> createMesh(StagingManager& stagingManager, const std::vector<BlockVertex>& vertices, const std::vector<uint32_t>& indices) const;
    };
}