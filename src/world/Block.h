#pragma once

#include <stdint.h>
#include <string>
#include <glm/glm.hpp>
#include <vector>

namespace vmc
{
    typedef uint8_t BlockId;

    struct Block
    {
        std::string name;
        std::vector<glm::vec2> uvs;
    };

    std::vector<Block> loadBlockDescriptions(const std::string& path);
}