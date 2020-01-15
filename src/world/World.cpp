#include "World.h"

namespace vmc
{
    glm::ivec2 getChunkCoordinate(const glm::ivec3& worldPosition)
    {
        glm::ivec2 coord;
        coord[0] = worldPosition.x < 0 ? (worldPosition.x + 1) / (int32_t)ChunkWidth - 1 : worldPosition.x / ChunkWidth;
        coord[1] = worldPosition.z < 0 ? (worldPosition.z + 1) / (int32_t)ChunkLength - 1 : worldPosition.z / ChunkLength;
        return coord;
    }

    World::World()
    {
    }

    void World::createChunk(const glm::ivec2& coords)
    {
        chunks.emplace(coords, Chunk());
    }

    std::unordered_map<glm::ivec2, Chunk>& World::getChunks()
    {
        return chunks;
    }

    const Chunk* World::getChunk(const glm::ivec3& worldPosition) const
    {
        auto it = chunks.find(getChunkCoordinate(worldPosition));
        return it == chunks.end() ? nullptr : &it->second;
    }
}
