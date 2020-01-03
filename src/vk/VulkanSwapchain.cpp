#include "VulkanSwapchain.h"
#include "common/Utils.h"
#include <stdexcept>

namespace vmc
{
	std::vector<VkSurfaceFormatKHR> getFormats(const VulkanDevice& device, VkSurfaceKHR surface)
	{
		uint32_t count;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device.getPhysicalDevice(), surface, &count, nullptr);

		std::vector<VkSurfaceFormatKHR> formats(count);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device.getPhysicalDevice(), surface, &count, formats.data());
		return formats;
	}

	std::vector<VkPresentModeKHR> getPresentModes(const VulkanDevice& device, VkSurfaceKHR surface)
	{
		uint32_t count;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device.getPhysicalDevice(), surface, &count, nullptr);

		std::vector<VkPresentModeKHR> presentModes(count);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device.getPhysicalDevice(), surface, &count, presentModes.data());
		return presentModes;
	}

	VkSurfaceFormatKHR chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		for (const auto& surfaceFormat : availableFormats) {
			if (surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR && surfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM) {
				return surfaceFormat;
			}
		}

		return availableFormats[0];
	}

	VkPresentModeKHR choosePresentMode(const std::vector<VkPresentModeKHR>& availableModes)
	{
		for (const auto& mode : availableModes) {
			if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
				return mode;
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D chooseExtent(const VkSurfaceCapabilitiesKHR& capabilities, uint32_t width, uint32_t height)
	{
		if (width == 0 || height == 0) {
			return capabilities.currentExtent;
		}

		VkExtent2D extent;
		extent.width = clamp(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		extent.height = clamp(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return extent;
	}

	VulkanSwapchain::VulkanSwapchain(const VulkanDevice& device, VkSurfaceKHR surface, uint32_t width, uint32_t height) :
		device(device),
		surface(surface)
	{
		VkSurfaceCapabilitiesKHR capabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.getPhysicalDevice(), surface, &capabilities);

		auto availableFormats = getFormats(device, surface);
		auto availablePresentModes = getPresentModes(device, surface);
		auto surfaceFormat = chooseSurfaceFormat(availableFormats);

		VkSwapchainCreateInfoKHR createInfo{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
		createInfo.surface = surface;
		createInfo.imageArrayLayers = 1;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		createInfo.imageExtent = chooseExtent(capabilities, width, height);
		createInfo.minImageCount = clamp(3u, capabilities.minImageCount, capabilities.maxImageCount);
		createInfo.presentMode = choosePresentMode(availablePresentModes);

		std::vector<uint32_t> queueFamilyIndices = { device.getGraphicsQueueFamilyIndex(), device.getPresentQueueFamilyIndex() };
		if (device.getGraphicsQueueFamilyIndex() != device.getPresentQueueFamilyIndex()) {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = queueFamilyIndices.size();
			createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
		}
		else {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}
		createInfo.preTransform = capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		if (vkCreateSwapchainKHR(device.getHandle(), &createInfo, nullptr, &handle) != VK_SUCCESS) {
			throw std::runtime_error("Cannot create swapchain.");
		}

		uint32_t imageCount;
		vkGetSwapchainImagesKHR(device.getHandle(), handle, &imageCount, nullptr);

		images.resize(imageCount);
		vkGetSwapchainImagesKHR(device.getHandle(), handle, &imageCount, images.data());

		format = surfaceFormat.format;
		extent = createInfo.imageExtent;
		presentMode = createInfo.presentMode;
		colorSpace = createInfo.imageColorSpace;
	}

	VulkanSwapchain::VulkanSwapchain(const VulkanSwapchain& oldSwapchain, uint32_t width, uint32_t height) :
		colorSpace(oldSwapchain.colorSpace),
		presentMode(oldSwapchain.presentMode),
		format(oldSwapchain.format),
		device(oldSwapchain.device),
		surface(oldSwapchain.surface)
	{
		VkSurfaceCapabilitiesKHR capabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.getPhysicalDevice(), surface, &capabilities);

		VkSwapchainCreateInfoKHR createInfo{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
		createInfo.surface = surface;
		createInfo.imageArrayLayers = 1;
		createInfo.imageFormat = format;
		createInfo.imageColorSpace = colorSpace;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		createInfo.imageExtent = chooseExtent(capabilities, width, height);
		createInfo.minImageCount = oldSwapchain.images.size();
		createInfo.presentMode = presentMode;
		createInfo.oldSwapchain = oldSwapchain.handle;
		std::vector<uint32_t> queueFamilyIndices = { device.getGraphicsQueueFamilyIndex(), device.getPresentQueueFamilyIndex() };
		if (device.getGraphicsQueueFamilyIndex() != device.getPresentQueueFamilyIndex()) {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = queueFamilyIndices.size();
			createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
		}
		else {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}
		createInfo.preTransform = capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.clipped = VK_TRUE;

		if (vkCreateSwapchainKHR(device.getHandle(), &createInfo, nullptr, &handle) != VK_SUCCESS) {
			throw std::runtime_error("Cannot create swapchain.");
		}

		uint32_t imageCount;
		vkGetSwapchainImagesKHR(device.getHandle(), handle, &imageCount, nullptr);

		images.resize(imageCount);
		vkGetSwapchainImagesKHR(device.getHandle(), handle, &imageCount, images.data());

		extent = createInfo.imageExtent;
	}

	VulkanSwapchain::VulkanSwapchain(VulkanSwapchain&& other) noexcept :
		handle(other.handle),
		device(other.device),
		images(std::move(other.images)),
		format(other.format),
		extent(other.extent),
		colorSpace(other.colorSpace),
		presentMode(other.presentMode)
	{
		other.handle = VK_NULL_HANDLE;
	}

	VulkanSwapchain::~VulkanSwapchain()
	{
		if (handle != VK_NULL_HANDLE) {
			vkDestroySwapchainKHR(device.getHandle(), handle, nullptr);
		}
	}

	VkSwapchainKHR VulkanSwapchain::getHandle() const
	{
		return handle;
	}

	const std::vector<VkImage>& VulkanSwapchain::getImages() const
	{
		return images;
	}

	VkFormat VulkanSwapchain::getFormat() const
	{
		return format;
	}

	VkExtent2D VulkanSwapchain::getExtent() const
	{
		return extent;
	}

	VkSurfaceKHR VulkanSwapchain::getSurface() const
	{
		return surface;
	}
}