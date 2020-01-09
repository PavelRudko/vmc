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

		VkCommandBufferAllocateInfo allocateInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
		allocateInfo.commandPool = commandPool;
		allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocateInfo.commandBufferCount = 1;

		if (vkAllocateCommandBuffers(device.getHandle(), &allocateInfo, &commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("Cannot allocate command buffer.");
		}
	}

	StagingManager::~StagingManager()
	{
		if (commandPool != VK_NULL_HANDLE) {
			vkDestroyCommandPool(device.getHandle(), commandPool, nullptr);
		}
	}

	void StagingManager::start()
	{
		vkResetCommandBuffer(commandBuffer, 0);

		VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		
		vkBeginCommandBuffer(commandBuffer, &beginInfo);
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

    void StagingManager::copyToImage(const void* data, VulkanImage& image, uint32_t width, uint32_t height)
    {
		VkDeviceSize size = width * height * 4;

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
		region.imageExtent.width = width;
		region.imageExtent.height = height;
		region.imageExtent.depth = 1;

		addLayoutTransition(image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		vkCmdCopyBufferToImage(commandBuffer, stagingBuffer.getHandle(), image.getHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
		addLayoutTransition(image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
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

	void StagingManager::addLayoutTransition(VulkanImage& image, VkImageLayout oldLayout, VkImageLayout newLayout)
	{
		VkImageMemoryBarrier barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image.getHandle();
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
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

		vkCmdPipelineBarrier(commandBuffer,
			sourceStage,
			destinationStage,
			0,
			0, nullptr, 
			0, nullptr,
			1, &barrier);
	}
}