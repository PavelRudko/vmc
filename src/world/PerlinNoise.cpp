#include "PerlinNoise.h"
#include <math.h>
#include <glm/gtc/constants.hpp>

namespace vmc
{
    PerlinNoise::PerlinNoise(int32_t seed) :
        seed(seed)
    {
    }

    float PerlinNoise::getValue(float x, float y) const
    {
        int32_t minX = (int32_t)floorf(x);
        int32_t maxX = minX + 1;
        int32_t minY = (int32_t)floorf(y);
        int32_t maxY = minY + 1;

        auto topLeft = randomDirection(minX, minY);
        auto topRight = randomDirection(maxX, minY);
        auto bottomLeft = randomDirection(minX, maxY);
        auto bottomRight = randomDirection(maxX, maxY);

        glm::vec2 point(x - minX, y - minY);
        return interpolate(point, topLeft, topRight, bottomLeft, bottomRight);
    }

    float PerlinNoise::random(int32_t x, int32_t y) const
    {
        x = x + seed + y * 57;
        x = (x << 13) ^ x;
        return (1.0 - ((x * (x * x * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0);
    }

    glm::vec2 PerlinNoise::randomDirection(int32_t x, int32_t y) const
    {
        float d = random(x, y) * glm::pi<float>();
        return glm::vec2(cosf(d), sinf(d));
    }

    float smoothStep(float x)
    {
        return x * x * x * (x * (x * 6 - 15) + 10);
    }

    float interpolateSmooth(float a, float b, float t)
    {
        t = smoothStep(t);
        return (1.0f - t) * a + t * b;
    }

    float PerlinNoise::interpolate(const glm::vec2& point, const glm::vec2& topLeft, const glm::vec2& topRight, const glm::vec2& bottomLeft, const glm::vec2& bottomRight) const
    {
        float tl = glm::dot(point, topLeft);
        float tr = glm::dot(point - glm::vec2(1, 0), topRight);
        float bl = glm::dot(point - glm::vec2(0, 1), bottomLeft);
        float br = glm::dot(point - glm::vec2(1, 1), bottomRight);

        float top = interpolateSmooth(tl, tr, point.x);
        float bottom = interpolateSmooth(bl, br, point.x);

        return interpolateSmooth(top, bottom, point.y);
    }
}