#pragma once

#include <vk/VulkanDevice.h>

namespace vmc
{
	class DescriptorSetLayout
	{
	public:
		DescriptorSetLayout(const VulkanDevice& device, const std::vector<VkDescriptorSetLayoutBinding>& bindings);

		DescriptorSetLayout(const DescriptorSetLayout&) = delete;

		DescriptorSetLayout(DescriptorSetLayout&& other) noexcept;

		~DescriptorSetLayout();

		DescriptorSetLayout& operator=(const DescriptorSetLayout&) = delete;

		DescriptorSetLayout& operator=(DescriptorSetLayout&&) = delete;

		VkDescriptorSetLayout getHandle() const;

	private:
		const VulkanDevice& device;

		VkDescriptorSetLayout handle = VK_NULL_HANDLE;
	};
}