#pragma once

#include <vk/VulkanBuffer.h>

namespace vmc
{
	const VkDeviceSize DefaultStagingBufferSize = 2 * 1024 * 1024;

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

		void copyToBuffer(void* data, VulkanBuffer& buffer, VkDeviceSize offset, VkDeviceSize size);

		void flush();

	private:
		const VulkanDevice& device;

		VulkanBuffer stagingBuffer;

		VkDeviceSize currentOffset = 0;

		VkCommandPool commandPool = VK_NULL_HANDLE;

		VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
	};
}