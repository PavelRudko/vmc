#pragma once

#include <vk/VulkanDevice.h>

namespace vmc
{
	class RenderPass
	{
	public:
		RenderPass(VulkanDevice& device, VkFormat colorFormat);

		RenderPass(const RenderPass&) = delete;

		RenderPass(RenderPass&&) = delete;

		~RenderPass();

		RenderPass& operator=(const RenderPass&) = delete;

		RenderPass& operator=(RenderPass&&) = delete;

		VkRenderPass getHandle() const;

	private:
		VulkanDevice& device;

		VkRenderPass handle = VK_NULL_HANDLE;
	};
}