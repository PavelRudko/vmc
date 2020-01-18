#pragma once

#include "Chunk.h"
#include "PerlinNoise.h"

namespace vmc
{
    class TerrainGenerator
    {
    public:
        TerrainGenerator(int32_t seed);

        TerrainGenerator(const TerrainGenerator&) = delete;

        TerrainGenerator(TerrainGenerator&& other) = delete;

        ~TerrainGenerator() = default;

        TerrainGenerator& operator=(const TerrainGenerator&) = delete;

        TerrainGenerator& operator=(TerrainGenerator&&) = delete;

        void generateChunk(Chunk& chunk, const glm::ivec2& chunkOffset) const;

    private:
        int32_t seed;

        PerlinNoise noise;
    };
}