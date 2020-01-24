#pragma once

#include "Chunk.h"
#include <unordered_map>
#include <glm/glm.hpp>
#include "TerrainGenerator.h"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/hash.hpp"

namespace vmc
{
    glm::ivec2 getChunkCoordinate(const glm::ivec3& worldPosition);

    class World
    {
    public:
        World(int32_t seed);

        World(const World&) = delete;

        World(World&& other) = delete;

        ~World() = default;

        World& operator=(const World&) = delete;

        World& operator=(World&&) = delete;

        std::unordered_map<glm::ivec2, Chunk>& getChunks();
        
        const Chunk* getChunk(const glm::ivec3& worldPosition) const;

        void preloadChunks(const glm::ivec3 center, int32_t radius);

        Chunk& generateChunk(const glm::ivec2& coordinate);

    private:
        std::unordered_map<glm::ivec2, Chunk> chunks;

        TerrainGenerator terrainGenerator;
    };
}