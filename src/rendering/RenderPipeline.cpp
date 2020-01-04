#include "RenderPipeline.h"
#include <stdexcept>

namespace vmc
{
	RenderPipeline::RenderPipeline(const VulkanDevice& device, const RenderPipelineDescription& description) :
		device(device)
	{
		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
		std::string entryPointName = "main";
		for (const auto& shaderModule : description.shaderModules) {
			VkPipelineShaderStageCreateInfo stageCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
			stageCreateInfo.stage = shaderModule.getStage();
			stageCreateInfo.module = shaderModule.getHandle();
			stageCreateInfo.pName = entryPointName.c_str();
			shaderStages.push_back(stageCreateInfo);
		}

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.pVertexBindingDescriptions = nullptr;
		vertexInputInfo.vertexAttributeDescriptionCount = 0;
		vertexInputInfo.pVertexAttributeDescriptions = nullptr;

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

		VkPipelineDynamicStateCreateInfo dynamicState{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
		dynamicState.dynamicStateCount = dynamicStates.size();
		dynamicState.pDynamicStates = dynamicStates.data();

		VkPipelineViewportStateCreateInfo viewportState { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		VkPipelineRasterizationStateCreateInfo rasterizer{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;

		VkPipelineMultisampleStateCreateInfo multisampling{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlending{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;

		VkPipelineLayoutCreateInfo layoutCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };

		if (vkCreatePipelineLayout(device.getHandle(), &layoutCreateInfo, nullptr, &layout) != VK_SUCCESS) {
			throw std::runtime_error("Cannot create pipeline layout.");
		}

		VkGraphicsPipelineCreateInfo createInfo{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
		createInfo.stageCount = shaderStages.size();
		createInfo.pStages = shaderStages.data();
		createInfo.pVertexInputState = &vertexInputInfo;
		createInfo.pInputAssemblyState = &inputAssembly;
		createInfo.pViewportState = &viewportState;
		createInfo.pDynamicState = &dynamicState;
		createInfo.pRasterizationState = &rasterizer;
		createInfo.pMultisampleState = &multisampling;
		createInfo.pColorBlendState = &colorBlending;
		createInfo.layout = layout;
		createInfo.renderPass = description.renderPass;
		createInfo.subpass = description.subpass;

		if (vkCreateGraphicsPipelines(device.getHandle(), VK_NULL_HANDLE, 1, &createInfo, nullptr, &handle) != VK_SUCCESS) {
			throw std::runtime_error("Cannot create pipeline.");
		}
	}

	RenderPipeline::~RenderPipeline()
	{
		if (handle != VK_NULL_HANDLE) {
			vkDestroyPipeline(device.getHandle(), handle, nullptr);
		}

		if (layout != VK_NULL_HANDLE) {
			vkDestroyPipelineLayout(device.getHandle(), layout, nullptr);
		}
	}

	VkPipeline RenderPipeline::getHandle() const
	{
		return handle;
	}

	VkPipelineLayout RenderPipeline::getLayout() const
	{
		return layout;
	}
}