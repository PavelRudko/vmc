#include "Mesh.h"
#include <vk/StagingManager.h>
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <world/World.h>

namespace vmc
{
    enum Faces
    {
        None = 0,
        Top = 1,
        Bottom = 2,
        Front = 4,
        Back = 8,
        Right = 16,
        Left = 32,
        All = Top | Bottom | Front | Back | Right | Left
    };

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

        Mesh buildChunkMesh(StagingManager& stagingManager, const World& world, const Chunk& chunk, const glm::ivec2& chunkCoordinate) const;

        Mesh buildBlockMesh(StagingManager& stagingManager, BlockId blockId) const;

    private:
        const VulkanDevice& device;

        const std::vector<Block>& blockDescriptions;

        void addCube(std::vector<BlockVertex>& vertices, std::vector<uint32_t>& indices, const std::vector<glm::vec2>& uvs, const glm::vec3& center = {0, 0, 0}, uint8_t visibleFaces = Faces::All) const;

        Mesh createMesh(StagingManager& stagingManager, const std::vector<BlockVertex>& vertices, const std::vector<uint32_t>& indices) const;
    };
}