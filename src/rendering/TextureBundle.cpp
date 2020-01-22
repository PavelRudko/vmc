#include "TextureBundle.h"
#include <common/Image.h>
#include <stdexcept>
#include <algorithm>

namespace vmc
{
    TextureBundle::TextureBundle(const VulkanDevice& device, const DescriptorSetLayout& textureLayout, StagingManager& stagingManager, uint32_t maxCount) :
        device(device),
        textureLayout(textureLayout),
        stagingManager(stagingManager)
    {
        VkSamplerCreateInfo samplerInfo{ VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
        samplerInfo.magFilter = VK_FILTER_NEAREST;
        samplerInfo.minFilter = VK_FILTER_NEAREST;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.minLod = 0;
        samplerInfo.maxLod = 4;
        samplerInfo.mipLodBias = -1.2f;

        if (vkCreateSampler(device.getHandle(), &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
            throw std::runtime_error("Cannot create sampler.");
        }

        descriptorPool = std::make_unique<DescriptorPool>(device, 0, 0, maxCount, maxCount);
    }

    TextureBundle::~TextureBundle()
    {
        if (sampler != VK_NULL_HANDLE) {
            vkDestroySampler(device.getHandle(), sampler, nullptr);
        }
    }

    void TextureBundle::add(const std::string& name, const std::string& path, uint32_t mipLevels)
    {
        Image rawImage("data/images/main_atlas.png");

        auto usageFlags = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        if (mipLevels > 1) {
            usageFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        }

        images.emplace_back(device,
            rawImage.getWidth(),
            rawImage.getHeight(),
            VK_FORMAT_R8G8B8A8_UNORM,
            VK_SAMPLE_COUNT_1_BIT, usageFlags,
            VMA_MEMORY_USAGE_GPU_ONLY, mipLevels);

        auto& image = images.back();
        stagingManager.start();
        stagingManager.copyToImage(rawImage.getData(), image, mipLevels > 1 ? VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        stagingManager.flush();

        if (mipLevels > 1) {
            stagingManager.startGraphics();
            stagingManager.generateMipmap(image);
            stagingManager.flushGraphics();
        }

        imageViews.emplace_back(device, image, VK_IMAGE_ASPECT_COLOR_BIT);
        auto& imageView = imageViews.back();

        auto descriptor = descriptorPool->allocate(textureLayout.getHandle());

        VkDescriptorImageInfo imageInfo = {};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = imageView.getHandle();
        imageInfo.sampler = sampler;

        VkWriteDescriptorSet samplerUpdateInfo{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
        samplerUpdateInfo.dstSet = descriptor;
        samplerUpdateInfo.dstBinding = 0;
        samplerUpdateInfo.dstArrayElement = 0;
        samplerUpdateInfo.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerUpdateInfo.descriptorCount = 1;
        samplerUpdateInfo.pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(device.getHandle(), 1, &samplerUpdateInfo, 0, nullptr);

        descriptors[name] = descriptor;
    }

    VkDescriptorSet TextureBundle::getDescriptor(const std::string& name) const
    {
        auto it = descriptors.find(name);
        if (it == descriptors.end()) {
            throw std::runtime_error("Texture with name " + name + " is not found in bundle.");
        }
        return it->second;
    }
}