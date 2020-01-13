#include "Block.h"
#include <json/json.h>
#include <fstream>
#include <stdexcept>

namespace vmc
{
    std::vector<glm::vec2> calculateNormalizedUVs(const std::vector<glm::vec2>& ranges, uint32_t textureWidth, uint32_t textureHeight)
    {
        std::vector<glm::vec2> uvs;
        for (uint32_t i = 0; i < ranges.size(); i += 2) {
            auto uvMin = ranges[i];
            auto uvMax = ranges[i + 1];

            uvMin.x /= textureWidth;
            uvMin.y /= textureHeight;

            uvMax.x /= textureWidth;
            uvMax.y /= textureHeight;

            uvs.push_back({ uvMin.x, uvMax.y });
            uvs.push_back({ uvMax.x, uvMax.y });
            uvs.push_back({ uvMax.x, uvMin.y });
            uvs.push_back({ uvMin.x, uvMin.y });
        }

        uint32_t baseIndex = uvs.size() - 4;
        uint32_t facesLeft = 6 - uvs.size() / 4;
        for (uint32_t i = 0; i < facesLeft; i++) {
            uvs.push_back(uvs[baseIndex + 0]);
            uvs.push_back(uvs[baseIndex + 1]);
            uvs.push_back(uvs[baseIndex + 2]);
            uvs.push_back(uvs[baseIndex + 3]);
        }

        return uvs;
    }

    void tryAddFaceUVs(Json::Value& element, std::vector<glm::vec2>& uvs)
    {
        if (element == 0) {
            return;
        }

        if (element.size() < 4) {
            throw std::runtime_error("Wrong UVs size");
        }

        uvs.push_back({ element[0].asUInt(),  element[1].asUInt() });
        uvs.push_back({ element[2].asUInt(),  element[3].asUInt() });
    }

    std::vector<Block> loadBlockDescriptions(const std::string& path)
    {
        std::vector<Block> blocks(std::numeric_limits<BlockId>().max());

        Json::Value root;
        std::ifstream file(path);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open block description file " + path + ".");
        }

        file >> root;
        for (const auto& element : root) {
            Block block;
            uint32_t id = element.get("id", 0).asUInt();
            if (id == 0) {
                throw std::runtime_error("Block id is missing.");
            }

            blocks[id].name = element.get("name", "").asString();

            std::vector<glm::vec2> uvs;
            auto uvsElement = element.get("uvs", 0);

            if (uvsElement == 0) {
                throw std::runtime_error("Block UVs are missing.");
            }

            tryAddFaceUVs(uvsElement.get("top", 0), uvs);
            tryAddFaceUVs(uvsElement.get("bottom", 0), uvs);
            tryAddFaceUVs(uvsElement.get("front", 0), uvs);
            tryAddFaceUVs(uvsElement.get("back", 0), uvs);
            tryAddFaceUVs(uvsElement.get("right", 0), uvs);
            tryAddFaceUVs(uvsElement.get("left", 0), uvs);

            if (uvs.size() < 2) {
                throw std::runtime_error("UVs are empty.");
            }

            blocks[id].uvs = calculateNormalizedUVs(uvs, 512, 512);
        }

        return blocks;
    }
}