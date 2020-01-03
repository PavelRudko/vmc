#pragma once

#include "VulkanDevice.h"

namespace vmc
{
	class VulkanShaderModule
	{
	public:
		VulkanShaderModule(const VulkanDevice& device, const std::vector<uint8_t>& data);

		VulkanShaderModule(const VulkanShaderModule&) = delete;

		VulkanShaderModule(VulkanShaderModule&& other) = delete;

		~VulkanShaderModule();

		VulkanShaderModule& operator=(const VulkanShaderModule&) = delete;

		VulkanShaderModule& operator=(VulkanShaderModule&&) = delete;

		VkShaderModule getHandle() const;

	private:
		const VulkanDevice& device;

		VkShaderModule handle = VK_NULL_HANDLE;
	};
}