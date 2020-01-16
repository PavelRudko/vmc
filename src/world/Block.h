#pragma once

#include <stdint.h>
#include <string>
#include <glm/glm.hpp>
#include <vector>

namespace vmc
{
    using BlockId = uint8_t;

    constexpr BlockId AirBlockId = 0;

    enum BlockShape
    {
        Cube = 1,
        Cross = 2
    };

    struct Block
    {
        std::string name;
        bool isOpaque;
        std::vector<glm::vec2> uvs;
        BlockShape shape;
        float width;
        float height;
    };

    std::vector<Block> loadBlockDescriptions(const std::string& path);
}