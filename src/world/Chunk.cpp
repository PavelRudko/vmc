#include "Chunk.h"

namespace vmc
{
    Chunk::Chunk()
    {
        size_t size = ChunkHeight * ChunkLength * ChunkWidth;
        blocks = new BlockId[size];
        memset(blocks, 0, size);
    }

    Chunk::Chunk(Chunk&& other) noexcept :
        blocks(other.blocks)
    {
        other.blocks = nullptr;
    }

    Chunk::~Chunk()
    {
        if (blocks) {
            delete[] blocks;
        }
    }

    BlockId Chunk::getBlock(uint32_t x, uint32_t y, uint32_t z) const
    {
        return blocks[getIndexInChunk(x, y, z)];
    }

    void Chunk::setBlock(uint32_t x, uint32_t y, uint32_t z, BlockId id)
    {
        if (y > maxHeight) {
            maxHeight = y;
        }
        blocks[getIndexInChunk(x, y, z)] = id;
    }

    uint32_t Chunk::getMaxHeight() const
    {
        return maxHeight;
    }

    BlockId* Chunk::getData()
    {
        return blocks;
    }

    const BlockId* Chunk::getData() const
    {
        return blocks;
    }
}