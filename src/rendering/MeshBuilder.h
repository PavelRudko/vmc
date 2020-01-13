#include "Mesh.h"
#include <vk/StagingManager.h>
#include <glm/glm.hpp>
#include <memory>
#include <string>

namespace vmc
{
    typedef uint8_t BlockId;

    struct Block
    {
        std::string name;
        std::vector<glm::vec2> uvs;
    };

    struct BlockVertex
    {
        glm::vec4 position;
        glm::vec2 uv;
    };

    class MeshBuilder
    {
    public:
        MeshBuilder(const VulkanDevice& device);

        MeshBuilder(const MeshBuilder&) = delete;

        MeshBuilder(Mesh&&) = delete;

        MeshBuilder& operator=(const MeshBuilder&) = delete;

        MeshBuilder& operator=(MeshBuilder&&) = delete;

        void loadBlockDescriptions(const std::string& path);

        std::unique_ptr<Mesh> buildBlockMesh(StagingManager& stagingManager, BlockId blockId) const;

    private:
        const VulkanDevice& device;

        std::vector<Block> blocks;

        void addCube(std::vector<BlockVertex>& vertices, std::vector<uint32_t>& indices, const std::vector<glm::vec2>& uvs) const;
    };
}