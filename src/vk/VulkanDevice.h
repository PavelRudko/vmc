#pragma once

#include <volk.h>
#include <vk_mem_alloc.h>
#include <vector>

namespace vmc
{
	class VulkanDevice
	{
	public:
		VulkanDevice(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, const std::vector<const char*> requiredDeviceExtensions);

		VulkanDevice(const VulkanDevice&) = delete;

		VulkanDevice(VulkanDevice&& other) noexcept;

		~VulkanDevice();

		VulkanDevice& operator=(const VulkanDevice&) = delete;

		VulkanDevice& operator=(VulkanDevice&&) = delete;

		VkQueue getPresentQueue() const;

		VkQueue getGraphicsQueue() const;

		VkQueue getTransferQueue() const;

		VkQueue getComputeQueue() const;

		uint32_t getPresentQueueFamilyIndex() const;

		uint32_t getGraphicsQueueFamilyIndex() const;

		uint32_t getComputeQueueFamilyIndex() const;

		uint32_t getTransferQueueFamilyIndex() const;

		VkDevice getHandle() const;

		VkPhysicalDevice getPhysicalDevice() const;

		VkSurfaceFormatKHR getSurfaceFormat() const;

		VkFormat getDepthFormat() const;
		
		VmaAllocator getMemoryAllocator() const;

		const VkPhysicalDeviceProperties& getProperties() const;

		void waitIdle() const;

	private:
		VkPhysicalDevice physicalDevice;

		VkDevice handle = VK_NULL_HANDLE;

		VkQueue presentQueue = VK_NULL_HANDLE;

		VkQueue graphicsQueue = VK_NULL_HANDLE;

		VkQueue transferQueue = VK_NULL_HANDLE;

		VkQueue computeQueue = VK_NULL_HANDLE;

		uint32_t presentQueueFamilyIndex = 0;

		uint32_t graphicsQueueFamilyIndex = 0;

		uint32_t transferQueueFamilyIndex = 0;

		uint32_t computeQueueFamilyIndex = 0;

		VkSurfaceFormatKHR surfaceFormat;

		VkFormat depthFormat;

		VmaAllocator memoryAllocator = VK_NULL_HANDLE;

		VkPhysicalDeviceProperties properties;
	};
}