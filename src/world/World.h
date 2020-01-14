#pragma once

#include "Chunk.h"
#include <unordered_map>
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/hash.hpp"

namespace vmc
{
    class World
    {
    public:
        World();

        World(const World&) = delete;

        World(World&& other) = delete;

        ~World() = default;

        World& operator=(const World&) = delete;

        World& operator=(World&&) = delete;

        void createChunk(const glm::ivec2& coords);

        std::unordered_map<glm::ivec2, Chunk>& getChunks();

    private:
        std::unordered_map<glm::ivec2, Chunk> chunks;
    };
}