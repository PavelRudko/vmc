#include "TerrainGenerator.h"

namespace vmc
{
    TerrainGenerator::TerrainGenerator(int32_t seed) :
        seed(seed),
        noise(seed)
    {
    }

    void TerrainGenerator::generateChunk(Chunk& chunk, const glm::ivec2& chunkOffset) const
    {
        int32_t maxHeight = 64;
        int32_t minHeight = 32;
        float gridSize = 64.0f;

        for (uint32_t z = 0; z < ChunkLength; z++) {
            for (uint32_t x = 0; x < ChunkWidth; x++) {

                int32_t offsetX = x + chunkOffset[0] * (int32_t)ChunkWidth;
                int32_t offsetZ = z + chunkOffset[1] * (int32_t)ChunkLength;

                float n = noise.getValue(offsetX / gridSize, offsetZ / gridSize);
                int32_t height = minHeight + (maxHeight - minHeight) * (0.5f + n * 0.5f);
                for (int32_t y = 0; y <= height; y++) {
                    chunk.setBlock(x, y, z, y == height ? 1 : 2);
                }
            }
        }

        //chunk.setBlock(10, 40, 10, 5);
        //chunk.setBlock(4, 40, 7, 5);
        //chunk.setBlock(14, 40, 3, 5);
    }
}