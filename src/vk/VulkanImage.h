#pragma once

#include <vk/VulkanDevice.h>

namespace vmc
{
	class VulkanImage
	{
	public:
		VulkanImage(const VulkanDevice& device, uint32_t width, uint32_t height, VkFormat format, VkSampleCountFlagBits samples, VkImageUsageFlags usage, VmaMemoryUsage memoryUsage);

		VulkanImage(const VulkanImage&) = delete;

		VulkanImage(VulkanImage&& other) noexcept;

		~VulkanImage();

		VulkanImage& operator=(const VulkanImage&) = delete;

		VulkanImage& operator=(VulkanImage&&) = delete;

		VkImage getHandle() const;

		VkFormat getFormat() const;

	private:
		const VulkanDevice& device;

		VmaAllocation allocation = VK_NULL_HANDLE;

		VkImage handle = VK_NULL_HANDLE;

		VkFormat format;
	};

	class VulkanImageView
	{
	public:
		VulkanImageView(const VulkanDevice& device, const VulkanImage& image);

		VulkanImageView(const VulkanImageView&) = delete;

		VulkanImageView(VulkanImageView&& other) noexcept;

		~VulkanImageView();

		VulkanImageView& operator=(const VulkanImageView&) = delete;

		VulkanImageView& operator=(VulkanImageView&&) = delete;

		VkImageView getHandle() const;

	private:
		const VulkanDevice& device;

		VkImageView handle = VK_NULL_HANDLE;
	};
}