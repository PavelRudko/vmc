#pragma once

#include "VulkanDevice.h"

namespace vmc
{
	class VulkanSwapchain
	{
	public:
		VulkanSwapchain(const VulkanDevice& device, VkSurfaceFormatKHR surfaceFormat, VkSurfaceKHR surface, uint32_t width, uint32_t height);

		VulkanSwapchain(const VulkanSwapchain& oldSwapchain, uint32_t width, uint32_t height);

		VulkanSwapchain(const VulkanSwapchain&) = delete;

		VulkanSwapchain(VulkanSwapchain&& other) noexcept;

		~VulkanSwapchain();

		VulkanSwapchain& operator=(const VulkanSwapchain&) = delete;

		VulkanSwapchain& operator=(VulkanSwapchain&&) = delete;

		VkSwapchainKHR getHandle() const;

		const std::vector<VkImage>& getImages() const;

		VkFormat getFormat() const;

		VkExtent2D getExtent() const;

		VkSurfaceKHR getSurface() const;

	private:
		const VulkanDevice& device;

		VkSwapchainKHR handle = VK_NULL_HANDLE;

		std::vector<VkImage> images;

		VkFormat format;

		VkExtent2D extent;

		VkSurfaceKHR surface = VK_NULL_HANDLE;

		VkColorSpaceKHR colorSpace;

		VkPresentModeKHR presentMode;
	};
}