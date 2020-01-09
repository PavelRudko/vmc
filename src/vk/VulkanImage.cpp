#include "VulkanImage.h"
#include <stdexcept>

namespace vmc
{
    VulkanImage::VulkanImage(const VulkanDevice& device, uint32_t width, uint32_t height, VkFormat format, VkSampleCountFlagBits samples, VkImageUsageFlags usage, VmaMemoryUsage memoryUsage) :
        device(device),
        format(format)
    {
        VkImageCreateInfo createInfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
        createInfo.usage = usage;
        createInfo.imageType = VK_IMAGE_TYPE_2D;
        createInfo.extent.width = width;
        createInfo.extent.height = height;
        createInfo.extent.depth = 1;
        createInfo.mipLevels = 1;
        createInfo.arrayLayers = 1;
        createInfo.format = format;
        createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        createInfo.usage = usage;
        createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.samples = samples;

        VmaAllocationCreateInfo memoryInfo{};
        memoryInfo.usage = memoryUsage;

        VmaAllocationInfo allocationInfo{};

        if (vmaCreateImage(device.getMemoryAllocator(), &createInfo, &memoryInfo, &handle, &allocation, &allocationInfo) != VK_SUCCESS) {
            throw std::runtime_error("Cannot create image.");
        }
    }

    VulkanImage::VulkanImage(VulkanImage&& other) noexcept :
        device(other.device),
        allocation(other.allocation),
        handle(other.handle),
        format(other.format)
    {
        other.handle = VK_NULL_HANDLE;
        other.allocation = VK_NULL_HANDLE;
    }

    VulkanImage::~VulkanImage()
    {
        if (allocation != VK_NULL_HANDLE && handle != VK_NULL_HANDLE) {
            vmaDestroyImage(device.getMemoryAllocator(), handle, allocation);
        }
    }

    VkImage VulkanImage::getHandle() const
    {
        return handle;
    }

    VkFormat VulkanImage::getFormat() const
    {
        return format;
    }

    VulkanImageView::VulkanImageView(const VulkanDevice& device, const VulkanImage& image) :
        device(device)
    {
        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = image.getHandle();
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = image.getFormat();
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device.getHandle(), &createInfo, nullptr, &handle) != VK_SUCCESS) {
            throw std::runtime_error("Cannot create image view.");
        }
    }

    VulkanImageView::VulkanImageView(VulkanImageView&& other) noexcept :
        device(other.device),
        handle(other.handle)
    {
        other.handle = VK_NULL_HANDLE;
    }

    VulkanImageView::~VulkanImageView()
    {
        if (handle != VK_NULL_HANDLE) {
            vkDestroyImageView(device.getHandle(), handle, nullptr);
        }
    }

    VkImageView VulkanImageView::getHandle() const
    {
        return handle;
    }
}