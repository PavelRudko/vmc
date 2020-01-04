#include "VulkanShaderModule.h"
#include <stdexcept>

namespace vmc
{
	VulkanShaderModule::VulkanShaderModule(const VulkanDevice& device, const std::vector<uint8_t>& data, VkShaderStageFlagBits stage) :
		stage(stage),
		device(device)
	{
		VkShaderModuleCreateInfo createInfo{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
		createInfo.codeSize = data.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(data.data());

		if (vkCreateShaderModule(device.getHandle(), &createInfo, nullptr, &handle) != VK_SUCCESS) {
			throw std::runtime_error("Cannot create shader module.");
		}
	}

	VulkanShaderModule::VulkanShaderModule(VulkanShaderModule&& other) noexcept :
		device(other.device),
		handle(other.handle),
		stage(other.stage)
	{
		other.handle = VK_NULL_HANDLE;
	}

	VulkanShaderModule::~VulkanShaderModule()
	{
		if (handle != VK_NULL_HANDLE) {
			vkDestroyShaderModule(device.getHandle(), handle, nullptr);
		}
	}

	VkShaderModule VulkanShaderModule::getHandle() const
	{
		return handle;
	}

	VkShaderStageFlagBits VulkanShaderModule::getStage() const
	{
		return stage;
	}

}
