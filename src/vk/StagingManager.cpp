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

	void StagingManager::copyToBuffer(void* data, VulkanBuffer& buffer, VkDeviceSize offset, VkDeviceSize size)
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
}