#pragma once

#include <vk/VulkanDevice.h>

namespace vmc
{
	class DescriptorPool
	{
	public:
		DescriptorPool(const VulkanDevice& device, uint32_t uniformCount, uint32_t uniformDynamicCount, uint32_t texturesCount, uint32_t setCount);

		DescriptorPool(const DescriptorPool&) = delete;

		DescriptorPool(DescriptorPool&& other) = delete;

		~DescriptorPool();

		DescriptorPool& operator=(const DescriptorPool&) = delete;

		DescriptorPool& operator=(DescriptorPool&&) = delete;

		VkDescriptorSet allocate(VkDescriptorSetLayout layout);

	private:
		const VulkanDevice& device;

		VkDescriptorPool handle = VK_NULL_HANDLE;
	};
}