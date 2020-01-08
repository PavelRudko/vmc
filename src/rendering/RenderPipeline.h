#pragma once

#include <vk/VulkanDevice.h>
#include <vk/ShaderModule.h>

namespace vmc
{
	struct RenderPipelineDescription
	{
		std::vector<VkVertexInputBindingDescription> vertexBindings;
		std::vector<VkVertexInputAttributeDescription> vertexAttributes;
		std::vector<VulkanShaderModule> shaderModules;
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
		VkRenderPass renderPass;
		uint32_t subpass;
	};

	class RenderPipeline
	{
	public:
		RenderPipeline(const VulkanDevice& device, const RenderPipelineDescription& description);

		RenderPipeline(const RenderPipeline&) = delete;

		RenderPipeline(RenderPipeline&&) = delete;

		~RenderPipeline();

		RenderPipeline& operator=(const RenderPipeline&) = delete;

		RenderPipeline& operator=(RenderPipeline&&) = delete;

		VkPipeline getHandle() const;

		VkPipelineLayout getLayout() const;

	private:
		const VulkanDevice& device;

		VkPipeline handle = VK_NULL_HANDLE;

		VkPipelineLayout layout = VK_NULL_HANDLE;
	};
}