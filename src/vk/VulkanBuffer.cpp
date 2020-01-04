#include "VulkanBuffer.h"
#include <stdexcept>

namespace vmc
{
	VulkanBuffer::VulkanBuffer(const VulkanDevice& device, VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags flags) :
		device(device),
		size(size)
	{
		VkBufferCreateInfo createInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		createInfo.usage = usage;
		createInfo.size = size;

		VmaAllocationCreateInfo memoryInfo{};
		memoryInfo.flags = flags;
		memoryInfo.usage = memoryUsage;

		VmaAllocationInfo allocationInfo{};

		if (vmaCreateBuffer(device.getMemoryAllocator(), &createInfo, &memoryInfo, &handle, &allocation, &allocationInfo) != VK_SUCCESS) {
			throw std::runtime_error("Cannot create buffer.");
		}
	}

	VulkanBuffer::VulkanBuffer(VulkanBuffer&& other) noexcept :
		device(other.device),
		size(other.size),
		allocation(other.allocation),
		handle(other.handle)
	{
		other.handle = VK_NULL_HANDLE;
		other.allocation = VK_NULL_HANDLE;
	}

	VulkanBuffer::~VulkanBuffer()
	{
		if (allocation != VK_NULL_HANDLE && handle != VK_NULL_HANDLE) {
			vmaDestroyBuffer(device.getMemoryAllocator(), handle, allocation);
		}
	}

	VkBuffer VulkanBuffer::getHandle() const
	{
		return handle;
	}

	VkDeviceSize VulkanBuffer::getSize() const
	{
		return size;
	}

	void* VulkanBuffer::map()
	{
		void* data;
		vmaMapMemory(device.getMemoryAllocator(), allocation, &data);
		return data;
	}

	void VulkanBuffer::unmap()
	{
		vmaUnmapMemory(device.getMemoryAllocator(), allocation);
	}

	void VulkanBuffer::flush()
	{
		vmaFlushAllocation(device.getMemoryAllocator(), allocation, 0, size);
	}
}