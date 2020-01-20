#pragma once

#include <vk/VulkanBuffer.h>
#include <vk/VulkanImage.h>

namespace vmc
{
	const VkDeviceSize DefaultStagingBufferSize = 32 * 1024 * 1024;

	class StagingManager
	{
	public:
		StagingManager(const VulkanDevice& device, VkDeviceSize size = DefaultStagingBufferSize);

		StagingManager(const StagingManager&) = delete;

		StagingManager(StagingManager&& other) = delete;

		~StagingManager();

		StagingManager& operator=(const StagingManager&) = delete;

		StagingManager& operator=(StagingManager&&) = delete;

		void start();

		void copyToBuffer(const void* data, VulkanBuffer& buffer, VkDeviceSize offset, VkDeviceSize size);

		void copyToImage(const void* data, VulkanImage& image, VkImageLayout finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		void flush();

        void startGraphics();

        void generateMipmap(VulkanImage& image);

        void flushGraphics();

	private:
		const VulkanDevice& device;

		VulkanBuffer stagingBuffer;

		VkDeviceSize currentOffset = 0;

		VkCommandPool commandPool = VK_NULL_HANDLE;

		VkCommandBuffer commandBuffer = VK_NULL_HANDLE;

        VkCommandPool graphicsCommandPool = VK_NULL_HANDLE;

        VkCommandBuffer graphicsCommandBuffer = VK_NULL_HANDLE;

		void addLayoutTransition(VkCommandBuffer commandBuffer, VulkanImage& image, VkImageLayout oldLayout, VkImageLayout newLayout);
	};
}