#pragma once

#include <vk/VulkanImage.h>
#include <vk/DescriptorSetLayout.h>
#include <unordered_map>
#include <vk/DescriptorPool.h>
#include <vk/StagingManager.h>
#include <memory>

namespace vmc
{
	const uint32_t DefaultMaxTextures = 24;

	class TextureBundle
	{
	public:
		TextureBundle(const VulkanDevice& device, const DescriptorSetLayout& textureLayout, StagingManager& stagingManager, uint32_t maxCount = DefaultMaxTextures);

		TextureBundle(const TextureBundle&) = delete;

		TextureBundle(TextureBundle&& other) = delete;

		~TextureBundle();

		TextureBundle& operator=(const TextureBundle&) = delete;

		TextureBundle& operator=(TextureBundle&&) = delete;

		void add(const std::string& name, const std::string& path);

		VkDescriptorSet getDescriptor(const std::string& name) const;

	private:
		const VulkanDevice& device;

		const DescriptorSetLayout& textureLayout;

		StagingManager& stagingManager;

		VkSampler sampler = VK_NULL_HANDLE;

		std::unique_ptr<DescriptorPool> descriptorPool;

		std::vector<VulkanImage> images;

		std::vector<VulkanImageView> imageViews;

		std::unordered_map<std::string, VkDescriptorSet> descriptors;
	};
}