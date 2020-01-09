#include "GameView.h"
#include <fstream>
#include <stdexcept>
#include <core/Application.h>
#include <vk/ShaderModule.h>
#include <glm/gtc/matrix_transform.hpp>
#include <common/Image.h>

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
		Image image("data/images/main_atlas.png");
		printf("W: %u, H: %u, C: %u\n", image.getWidth(), image.getHeight(), image.getChannels());

		initPipeline();
		initBuffers();
	}

	GameView::~GameView()
	{
	}

	void GameView::update(float timeDelta)
	{
	}

	void GameView::render(RenderContext& renderContext)
	{
		auto& uniform = renderContext.getMVPUniform();
		auto uniformDescriptorSet = uniform.getDescriptorSet();

		auto viewMatrix = glm::lookAt(glm::vec3(0, 0, 2.0f), glm::vec3(0, 0, 0), glm::vec3(0, 1.0f, 0));
		auto projectionMatrix = glm::perspective(glm::pi<float>() / 2.0f, (float)renderContext.getWidth() / renderContext.getHeight(), 0.01f, 1000.0f);

		auto commandBuffer = renderContext.startFrame({ 0.8f, 0.9f, 1.0f, 1.0f });

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, defaultPipeline->getHandle());

		for (uint32_t i = 0; i < 2; i++) {
			auto modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(i == 0 ? -1 : 1, 0, 0));
			auto mvp = projectionMatrix * viewMatrix * modelMatrix;

			uint32_t uniformOffset = uniform.pushData(&mvp);
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, defaultPipeline->getLayout(), 0, 1, &uniformDescriptorSet, 1, &uniformOffset);

			VkBuffer vertexBufferHandle = vertexBuffer->getHandle();
			VkDeviceSize offset = 0;
			vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBufferHandle, &offset);

			vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getHandle(), 0, VK_INDEX_TYPE_UINT32);

			vkCmdDrawIndexed(commandBuffer, 6, 1, 0, 0, 0);
		}

		renderContext.endFrame();
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

		pipelineDescription.descriptorSetLayouts.push_back(application.getMVPLayout().getHandle());

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
	}
}

