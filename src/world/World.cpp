#include "World.h"

namespace vmc
{
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
}
