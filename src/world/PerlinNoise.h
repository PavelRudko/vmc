#pragma once

#include <stdint.h>
#include <glm/glm.hpp>

namespace vmc
{
    class PerlinNoise
    {
    public:
        PerlinNoise(int32_t seed);
        float getValue(float x, float y) const;

    private:
        int32_t seed;

        float random(int32_t x, int32_t y) const;
        glm::vec2 randomDirection(int32_t x, int32_t y) const;
        float interpolate(const glm::vec2& point, const glm::vec2& topLeft, const glm::vec2& topRight, const glm::vec2& bottomLeft, const glm::vec2& bottomRight) const;
    };
}