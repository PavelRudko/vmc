#include "Chunk.h"

namespace vmc
{
    Chunk::Chunk()
    {
        memset(blocks, 0, sizeof(blocks));
    }

    BlockId Chunk::getBlock(uint32_t x, uint32_t y, uint32_t z) const
    {
        return blocks[y * ChunkLength * ChunkWidth + z * ChunkWidth + x];
    }

    void Chunk::setBlock(uint32_t x, uint32_t y, uint32_t z, BlockId id)
    {
        blocks[y * ChunkLength * ChunkWidth + z * ChunkWidth + x] = id;
    }

    BlockId* Chunk::getData()
    {
        return blocks;
    }
}