#pragma once

#include "Block.h"

namespace vmc
{
    constexpr uint32_t ChunkWidth = 16;
    constexpr uint32_t ChunkLength = 16;
    constexpr uint32_t ChunkHeight = 256;

    inline bool isOutOfChunkBounds(const glm::ivec3& position)
    {
        return position.x < 0 || position.x >= ChunkWidth ||
            position.y < 0 || position.y >= ChunkHeight ||
            position.z < 0 || position.z >= ChunkLength;
    }

    inline size_t getIndexInChunk(uint32_t x, uint32_t y, uint32_t z)
    {
        return y * ChunkLength* ChunkWidth + z * ChunkWidth + x;
    }

    class Chunk
    {
    public:
        Chunk();

        Chunk(const Chunk&) = delete;

        Chunk(Chunk&& other) noexcept;

        ~Chunk();

        Chunk& operator=(const Chunk&) = delete;

        Chunk& operator=(Chunk&&) = delete;

        BlockId getBlock(uint32_t x, uint32_t y, uint32_t z) const;

        void setBlock(uint32_t x, uint32_t y, uint32_t z, BlockId id);

        uint32_t getMaxHeight() const;

        BlockId* getData();

        const BlockId* getData() const;

    private:
        BlockId* blocks = nullptr;

        uint32_t maxHeight = 0;
    };
}