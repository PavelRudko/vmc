#pragma once

#include "Block.h"

namespace vmc
{
    constexpr uint32_t ChunkWidth = 16;
    constexpr uint32_t ChunkLength = 16;
    constexpr uint32_t ChunkHeight = 256;

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

        BlockId* getData();

    private:
        BlockId* blocks = nullptr;
    };
}