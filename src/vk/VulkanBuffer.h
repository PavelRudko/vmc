#pragma once

#include <vk/VulkanDevice.h>

namespace vmc
{
	class VulkanBuffer
	{
	public:
		VulkanBuffer(const VulkanDevice& device, VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags flags = 0);

		VulkanBuffer(const VulkanBuffer&) = delete;

		VulkanBuffer(VulkanBuffer&& other) noexcept;

		~VulkanBuffer();

		VulkanBuffer& operator=(const VulkanBuffer&) = delete;

		VulkanBuffer& operator=(VulkanBuffer&&) = delete;

		VkBuffer getHandle() const;

		VkDeviceSize getSize() const;

		void* map();

		void unmap();

		void flush();

		void flush(VkDeviceSize offset, VkDeviceSize size);

		void copyFrom(void* src);

		void copyFrom(void* src, VkDeviceSize offset, VkDeviceSize size);

	private:
		const VulkanDevice& device;

		VkDeviceSize size = 0;

		VmaAllocation allocation = VK_NULL_HANDLE;

		VkBuffer handle = VK_NULL_HANDLE;
	};
}