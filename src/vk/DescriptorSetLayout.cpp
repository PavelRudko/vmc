#include "DescriptorSetLayout.h"
#include <stdexcept>

namespace vmc
{
	DescriptorSetLayout::DescriptorSetLayout(const VulkanDevice& device, const std::vector<VkDescriptorSetLayoutBinding>& bindings) :
		device(device)
	{
		VkDescriptorSetLayoutCreateInfo createInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
		createInfo.bindingCount = bindings.size();
		createInfo.pBindings = bindings.data();

		if (vkCreateDescriptorSetLayout(device.getHandle(), &createInfo, nullptr, &handle) != VK_SUCCESS) {
			throw std::runtime_error("Cannot create descriptor set layout.");
		}
	}

	DescriptorSetLayout::DescriptorSetLayout(DescriptorSetLayout&& other) noexcept :
		device(other.device),
		handle(other.handle)
	{
		other.handle = VK_NULL_HANDLE;
	}

	DescriptorSetLayout::~DescriptorSetLayout()
	{
		if (handle != VK_NULL_HANDLE) {
			vkDestroyDescriptorSetLayout(device.getHandle(), handle, nullptr);
		}
	}

	VkDescriptorSetLayout DescriptorSetLayout::getHandle() const
	{
		return handle;
	}
}