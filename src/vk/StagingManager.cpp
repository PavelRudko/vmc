#include "StagingManager.h"
#include <stdexcept>

namespace vmc
{
	StagingManager::StagingManager(const VulkanDevice& device, VkDeviceSize size) :
		stagingBuffer(device, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY),
		device(device)
	{
		VkCommandPoolCreateInfo poolCreateInfo{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
		poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolCreateInfo.queueFamilyIndex = device.getTransferQueueFamilyIndex();

		if (vkCreateCommandPool(device.getHandle(), &poolCreateInfo, nullptr, &commandPool) != VK_SUCCESS) {
			throw std::runtime_error("Cannot create command pool.");
		}

        poolCreateInfo.queueFamilyIndex = device.getGraphicsQueueFamilyIndex();
        if (vkCreateCommandPool(device.getHandle(), &poolCreateInfo, nullptr, &graphicsCommandPool) != VK_SUCCESS) {
            throw std::runtime_error("Cannot create command pool.");
        }

		VkCommandBufferAllocateInfo allocateInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
		allocateInfo.commandPool = commandPool;
		allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocateInfo.commandBufferCount = 1;

		if (vkAllocateCommandBuffers(device.getHandle(), &allocateInfo, &commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("Cannot allocate command buffer.");
		}

        allocateInfo.commandPool = graphicsCommandPool;
        if (vkAllocateCommandBuffers(device.getHandle(), &allocateInfo, &graphicsCommandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("Cannot allocate command buffer.");
        }
	}

	StagingManager::~StagingManager()
	{
		if (commandPool != VK_NULL_HANDLE) {
			vkDestroyCommandPool(device.getHandle(), commandPool, nullptr);
		}

        if (graphicsCommandPool != VK_NULL_HANDLE) {
            vkDestroyCommandPool(device.getHandle(), graphicsCommandPool, nullptr);
        }
	}

	void StagingManager::copyToBuffer(const void* data, VulkanBuffer& buffer, VkDeviceSize offset, VkDeviceSize size)
	{
		if (currentOffset + size > stagingBuffer.getSize()) {
			throw std::runtime_error("Staging buffer is too small.");
		}

		stagingBuffer.copyFrom(data, currentOffset, size);

		VkBufferCopy region;
		region.size = size;
		region.dstOffset = offset;
		region.srcOffset = currentOffset;
		vkCmdCopyBuffer(commandBuffer, stagingBuffer.getHandle(), buffer.getHandle(), 1, &region);

		currentOffset += size;
	}

    void StagingManager::copyToImage(const void* data, VulkanImage& image, VkImageLayout finalLayout)
    {
		VkDeviceSize size = image.getWidth() * image.getWidth() * 4;

		if (currentOffset + size > stagingBuffer.getSize()) {
			throw std::runtime_error("Staging buffer is too small.");
		}

		stagingBuffer.copyFrom(data, currentOffset, size);

		VkBufferImageCopy region{};
		region.bufferOffset = currentOffset;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = { 0, 0, 0 };
		region.imageExtent.width = image.getWidth();
		region.imageExtent.height = image.getWidth();
		region.imageExtent.depth = 1;

		addLayoutTransition(commandBuffer, image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		vkCmdCopyBufferToImage(commandBuffer, stagingBuffer.getHandle(), image.getHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
        if (finalLayout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            addLayoutTransition(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, finalLayout);
        }
    }

    void StagingManager::generateMipmap(VulkanImage& image)
    {
        VkImageMemoryBarrier barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
        barrier.image = image.getHandle();
        barrier.srcQueueFamilyIndex = device.getGraphicsQueueFamilyIndex();
        barrier.dstQueueFamilyIndex = device.getGraphicsQueueFamilyIndex();
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

        int32_t mipWidth = image.getWidth();
        int32_t mipHeight = image.getHeight();

        for (uint32_t i = 1; i < image.getMipLevels(); i++) {
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.subresourceRange.baseMipLevel = i - 1;

            vkCmdPipelineBarrier(graphicsCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

            VkImageBlit blit{};
            blit.srcOffsets[0] = { 0, 0, 0 };
            blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
            blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.srcSubresource.baseArrayLayer = 0;
            blit.srcSubresource.layerCount = 1;
            blit.srcSubresource.mipLevel = i - 1;
            
            mipWidth /= 2;
            mipHeight /= 2;

            blit.dstOffsets[0] = { 0, 0, 0 };
            blit.dstOffsets[1] = { mipWidth, mipHeight, 1 };
            blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.dstSubresource.baseArrayLayer = 0;
            blit.dstSubresource.layerCount = 1;
            blit.dstSubresource.mipLevel = i;

            vkCmdBlitImage(graphicsCommandBuffer, image.getHandle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image.getHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);
        }

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.subresourceRange.baseMipLevel = image.getMipLevels() - 1;

        vkCmdPipelineBarrier(graphicsCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

        addLayoutTransition(graphicsCommandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }

    void StagingManager::start()
    {
        vkResetCommandBuffer(commandBuffer, 0);

        VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);
    }

	void StagingManager::flush()
	{
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(device.getTransferQueue(), 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(device.getTransferQueue());

		currentOffset = 0;
	}

    void StagingManager::startGraphics()
    {
        vkResetCommandBuffer(graphicsCommandBuffer, 0);

        VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(graphicsCommandBuffer, &beginInfo);
    }

    void StagingManager::flushGraphics()
    {
        vkEndCommandBuffer(graphicsCommandBuffer);

        VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &graphicsCommandBuffer;

        vkQueueSubmit(device.getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(device.getGraphicsQueue());
    } 

	void StagingManager::addLayoutTransition(VkCommandBuffer commandBuffer, VulkanImage& image, VkImageLayout oldLayout, VkImageLayout newLayout)
	{
		VkImageMemoryBarrier barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image.getHandle();
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = image.getMipLevels();
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		VkPipelineStageFlags sourceStage = 0;
		VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			barrier.srcAccessMask = 0;
			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }

		vkCmdPipelineBarrier(commandBuffer,
			sourceStage,
			destinationStage,
			0,
			0, nullptr, 
			0, nullptr,
			1, &barrier);
	}
}