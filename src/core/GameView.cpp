#include "GameView.h"
#include <fstream>
#include <stdexcept>
#include <core/Application.h>
#include <vk/VulkanShaderModule.h>

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
		auto commandBuffer = renderContext.startFrame({ 0.8f, 0.9f, 1.0f, 1.0f });
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, defaultPipeline->getHandle());

		VkBuffer vertexBufferHandle = vertexBuffer->getHandle();
		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBufferHandle, &offset);

		vkCmdDraw(commandBuffer, 3, 1, 0, 0);
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

		defaultPipeline = std::make_unique<RenderPipeline>(application.getDevice(), pipelineDescription);
	}

	void GameView::initBuffers()
	{
		std::vector<ColorVertex> vertices = {
			{ {0.0, -0.5, 0, 1}, {1.0, 0.0, 0.0, 0} },
			{ {0.5, 0.5, 0, 1},  {0.0, 1.0, 0.0, 0} },
			{ {-0.5, 0.5, 0, 1}, {0.0, 0.0, 1.0, 0} },
		};

		vertexBuffer = std::make_unique<VulkanBuffer>(application.getDevice(), vertices.size() * sizeof(ColorVertex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
		auto bufferData = vertexBuffer->map();
		memcpy(bufferData, vertices.data(), vertices.size() * sizeof(ColorVertex));
		vertexBuffer->unmap();
		vertexBuffer->flush();


	}
}

