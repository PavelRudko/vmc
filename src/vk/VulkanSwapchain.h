#pragma once

#include "VulkanDevice.h"

namespace vmc
{
	class VulkanSwapchain
	{
	public:
		VulkanSwapchain(const VulkanDevice& device, VkSurfaceKHR surface, uint32_t width, uint32_t height);

		VulkanSwapchain(const VulkanSwapchain&) = delete;

		VulkanSwapchain(VulkanSwapchain&& other) noexcept;

		~VulkanSwapchain();

		VulkanSwapchain& operator=(const VulkanSwapchain&) = delete;

		VulkanSwapchain& operator=(VulkanSwapchain&&) = delete;

		VkSwapchainKHR getHandle() const;

	private:
		const VulkanDevice& device;

		VkSwapchainKHR handle = VK_NULL_HANDLE;
	};
}