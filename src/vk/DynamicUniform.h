#pragma once

#include <vk/DescriptorPool.h>
#include <vk/VulkanBuffer.h>

namespace vmc
{
	class DynamicUniform
	{
	public:
		DynamicUniform(const VulkanDevice& device, DescriptorPool& descriptorPool, VkDescriptorSetLayout descriptorSetLayout, VkDeviceSize elementSize, uint32_t maxElements);

		DynamicUniform(const DynamicUniform&) = delete;

		DynamicUniform(DynamicUniform&& other) noexcept;

		~DynamicUniform();

		DynamicUniform& operator=(const DynamicUniform&) = delete;

		DynamicUniform& operator=(DynamicUniform&&) = delete;

		void reset();

		VkDeviceSize pushData(const void* data);

		void flush();

		VkDescriptorSet getDescriptorSet() const;

	private:
		VkDescriptorSet descriptorSet = VK_NULL_HANDLE;

		VulkanBuffer buffer;

		void* mappedData = nullptr;

		VkDeviceSize offset = 0;

		VkDeviceSize elementSize;

		VkDeviceSize alignment;
	};
}