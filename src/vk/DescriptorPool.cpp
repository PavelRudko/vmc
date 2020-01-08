#include "DescriptorPool.h"
#include <stdexcept>

namespace vmc
{
	DescriptorPool::DescriptorPool(const VulkanDevice& device, uint32_t uniformCount, uint32_t uniformDynamicCount, uint32_t texturesCount, uint32_t setCount) :
		device(device)
	{
		std::vector<VkDescriptorPoolSize> poolSizes;

		if (uniformCount > 0) {
			VkDescriptorPoolSize poolSize{};
			poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			poolSize.descriptorCount = uniformCount;
			poolSizes.push_back(poolSize);
		}

		if (uniformDynamicCount > 0) {
			VkDescriptorPoolSize poolSize{};
			poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
			poolSize.descriptorCount = uniformDynamicCount;
			poolSizes.push_back(poolSize);
		}

		if (texturesCount > 0) {
			VkDescriptorPoolSize poolSize{};
			poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			poolSize.descriptorCount = texturesCount;
			poolSizes.push_back(poolSize);
		}

		VkDescriptorPoolCreateInfo createInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
		createInfo.maxSets = setCount;
		createInfo.poolSizeCount = poolSizes.size();
		createInfo.pPoolSizes = poolSizes.data();

		if (vkCreateDescriptorPool(device.getHandle(), &createInfo, nullptr, &handle) != VK_SUCCESS) {
			throw std::runtime_error("Cannot create descriptor pool.");
		}
	}

	DescriptorPool::~DescriptorPool()
	{
		if (handle != VK_NULL_HANDLE) {
			vkDestroyDescriptorPool(device.getHandle(), handle, nullptr);
		}
	}

	VkDescriptorSet DescriptorPool::allocate(VkDescriptorSetLayout layout)
	{
		VkDescriptorSet descriptorSet = VK_NULL_HANDLE;

		VkDescriptorSetAllocateInfo allocateInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
		allocateInfo.descriptorPool = handle;
		allocateInfo.descriptorSetCount = 1;
		allocateInfo.pSetLayouts = &layout;

		if (vkAllocateDescriptorSets(device.getHandle(), &allocateInfo, &descriptorSet) != VK_SUCCESS) {
			throw std::runtime_error("Cannot allocate descriptor set.");
		}

		return descriptorSet;
	}
}