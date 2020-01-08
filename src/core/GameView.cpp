#include "GameView.h"
#include <fstream>
#include <stdexcept>
#include <core/Application.h>
#include <vk/VulkanShaderModule.h>
#include <glm/gtc/matrix_transform.hpp>

namespace vmc
{
	std::vector<uint8_t> readFile(const std::string& path)
	{
		std::ifstream file(path, std::ios::binary | std::ios::ate);
		if (!file.is_open()) {
			throw std::runtime_error("Cannot open file " + path + ".");
		}

		size_t size = file.tellg();
		std::vector<uint8_t> data(size);
		file.seekg(0);
		file.read((char*)data.data(), size);
		file.close();

		return data;
	}

	GameView::GameView(Application& application) :
		View(application)
	{
		initDescriptorSetLayout();
		initPipeline();
		initBuffers();
		initDescriptorPool();
	}

	GameView::~GameView()
	{
		if (descriptorPool != VK_NULL_HANDLE) {
			vkDestroyDescriptorPool(application.getDevice().getHandle(), descriptorPool, nullptr);
		}

		if (descriptorSetLayout != VK_NULL_HANDLE) {
			vkDestroyDescriptorSetLayout(application.getDevice().getHandle(), descriptorSetLayout, nullptr);
		}
	}

	void GameView::update(float timeDelta)
	{
		static float totalTime = 0;
		totalTime += timeDelta;

		uniforms.model = glm::rotate(glm::mat4(1.0f), totalTime * 0.01f, glm::vec3(0, 1, 0));
		uniforms.view = glm::lookAt(glm::vec3(0, 0, 2.0f), glm::vec3(0, 0, 0), glm::vec3(0, 1.0f, 0));
	}

	void GameView::render(RenderContext& renderContext)
	{
		uint32_t currentUniformIndex = 0; 

		uniforms.projection = glm::perspective(glm::pi<float>() / 2.0f, (float)renderContext.getWidth() / renderContext.getHeight(), 0.01f, 1000.0f);
		uniformBuffers[currentUniformIndex].copyFrom(&uniforms);

		auto commandBuffer = renderContext.startFrame({ 0.8f, 0.9f, 1.0f, 1.0f });
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, defaultPipeline->getHandle());

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, defaultPipeline->getLayout(), 0, 1, &descriptorSets[currentUniformIndex], 0, nullptr);

		VkBuffer vertexBufferHandle = vertexBuffer->getHandle();
		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBufferHandle, &offset);

		vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getHandle(), 0, VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(commandBuffer, 6, 1, 0, 0, 0);
		renderContext.endFrame();

		currentUniformIndex++;
	}

	void GameView::initDescriptorSetLayout()
	{
		VkDescriptorSetLayoutBinding uniformDataBinding{};
		uniformDataBinding.binding = 0;
		uniformDataBinding.descriptorCount = 1;
		uniformDataBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uniformDataBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkDescriptorSetLayoutCreateInfo createInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
		createInfo.bindingCount = 1;
		createInfo.pBindings = &uniformDataBinding;

		if (vkCreateDescriptorSetLayout(application.getDevice().getHandle(), &createInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("Cannot create descriptor set layout.");
		}
	}

	void GameView::initDescriptorPool()
	{
		VkDescriptorPoolSize poolSize{};
		poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSize.descriptorCount = uniformBuffers.size();

		VkDescriptorPoolCreateInfo createInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
		createInfo.maxSets = uniformBuffers.size();
		createInfo.poolSizeCount = 1;
		createInfo.pPoolSizes = &poolSize;

		if (vkCreateDescriptorPool(application.getDevice().getHandle(), &createInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
			throw std::runtime_error("Cannot create descriptor pool.");
		}

		descriptorSets.resize(uniformBuffers.size(), VK_NULL_HANDLE);
		std::vector<VkDescriptorSetLayout> layouts(descriptorSets.size(), descriptorSetLayout);

		VkDescriptorSetAllocateInfo allocateInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
		allocateInfo.descriptorPool = descriptorPool;
		allocateInfo.descriptorSetCount = descriptorSets.size();
		allocateInfo.pSetLayouts = layouts.data();

		if (vkAllocateDescriptorSets(application.getDevice().getHandle(), &allocateInfo, descriptorSets.data()) != VK_SUCCESS) {
			throw std::runtime_error("Cannot allocate descriptor sets.");
		}

		for (uint32_t i = 0; i < descriptorSets.size(); i++) {
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = uniformBuffers[i].getHandle();
			bufferInfo.offset = 0;
			bufferInfo.range = uniformBuffers[i].getSize();

			VkWriteDescriptorSet writeInfo{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
			writeInfo.descriptorCount = 1;
			writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			writeInfo.dstBinding = 0;
			writeInfo.pBufferInfo = &bufferInfo;
			writeInfo.dstSet = descriptorSets[i];

			vkUpdateDescriptorSets(application.getDevice().getHandle(), 1, &writeInfo, 0, nullptr);
		}
	}

	void GameView::initPipeline()
	{
		auto vertexShaderData = readFile("data/shaders/default.vert.spv");
		auto fragmentShaderData = readFile("data/shaders/default.frag.spv");

		VulkanShaderModule vertexShader(application.getDevice(), vertexShaderData, VK_SHADER_STAGE_VERTEX_BIT);
		VulkanShaderModule fragmentShader(application.getDevice(), fragmentShaderData, VK_SHADER_STAGE_FRAGMENT_BIT);

		VkVertexInputBindingDescription colorVertexBinding{};
		colorVertexBinding.binding = 0;
		colorVertexBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		colorVertexBinding.stride = sizeof(ColorVertex);

		VkVertexInputAttributeDescription positionAttribute{};
		positionAttribute.binding = 0;
		positionAttribute.location = 0;
		positionAttribute.format = VK_FORMAT_R32G32B32A32_SFLOAT;
		positionAttribute.offset = offsetof(ColorVertex, position);

		VkVertexInputAttributeDescription colorAttribute{};
		colorAttribute.binding = 0;
		colorAttribute.location = 1;
		colorAttribute.format = VK_FORMAT_R32G32B32A32_SFLOAT;
		colorAttribute.offset = offsetof(ColorVertex, color);

		RenderPipelineDescription pipelineDescription;
		pipelineDescription.renderPass = application.getRenderPass().getHandle();
		pipelineDescription.subpass = 0;

		pipelineDescription.shaderModules.push_back(std::move(vertexShader));
		pipelineDescription.shaderModules.push_back(std::move(fragmentShader));
		
		pipelineDescription.vertexBindings.push_back(colorVertexBinding);
		pipelineDescription.vertexAttributes.push_back(positionAttribute);
		pipelineDescription.vertexAttributes.push_back(colorAttribute);

		pipelineDescription.descriptorSetLayouts.push_back(descriptorSetLayout);

		defaultPipeline = std::make_unique<RenderPipeline>(application.getDevice(), pipelineDescription);
	}

	void GameView::initBuffers()
	{
		auto& stagingManager = application.getStagingManager();

		std::vector<ColorVertex> vertices = {
			{{-0.5f, -0.5f, 0, 1}, {1.0f, 0.0f, 0.0f, 1}},
	        {{0.5f, -0.5f, 0, 1},  {0.0f, 1.0f, 0.0f, 1}},
	        {{0.5f, 0.5f, 0, 1},   {0.0f, 0.0f, 1.0f, 1}},
	        {{-0.5f, 0.5f, 0, 1},  {1.0f, 1.0f, 1.0f, 1}}
		};

		std::vector<uint32_t> indices = {
			0, 1, 2, 2, 3, 0
		};

		vertexBuffer = std::make_unique<VulkanBuffer>(application.getDevice(), vertices.size() * sizeof(ColorVertex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
		indexBuffer = std::make_unique<VulkanBuffer>(application.getDevice(), indices.size() * sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

		stagingManager.start();
		stagingManager.copyToBuffer(vertices.data(), *vertexBuffer, 0, vertexBuffer->getSize());
		stagingManager.copyToBuffer(indices.data(), *indexBuffer, 0, indexBuffer->getSize());
		stagingManager.flush();

		for (uint32_t i = 0; i < 3; i++) {
			uniformBuffers.push_back(VulkanBuffer(application.getDevice(), sizeof(UniformData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_ONLY));
		}
	}
}

