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

        void addAdjascent(const glm::ivec3& position, const Chunk& chunk, std::vector<uint8_t>& chunkFaces) const;

        void addBoundaryBlock(const glm::ivec3& position, const World& world, const Chunk& chunk, std::vector<uint8_t>& chunkFaces, const glm::ivec2& chunkCoordinate) const;

        void addTransparentBlock(const glm::ivec3& position, const Chunk& chunk, std::vector<uint8_t>& chunkFaces) const;

        Mesh createMesh(StagingManager& stagingManager, const std::vector<BlockVertex>& vertices, const std::vector<uint32_t>& indices) const;

        bool isOpaque(BlockId id) const;
    };
}