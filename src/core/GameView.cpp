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
		auto vertexShaderData = readFile("data/shaders/default.vert.spv");
		auto fragmentShaderData = readFile("data/shaders/default.frag.spv");

		VulkanShaderModule vertexShader(application.getDevice(), vertexShaderData, VK_SHADER_STAGE_VERTEX_BIT);
		VulkanShaderModule fragmentShader(application.getDevice(), fragmentShaderData, VK_SHADER_STAGE_FRAGMENT_BIT);

		RenderPipelineDescription pipelineDescription;
		pipelineDescription.shaderModules.push_back(std::move(vertexShader));
		pipelineDescription.shaderModules.push_back(std::move(fragmentShader));
		pipelineDescription.renderPass = application.getRenderPass().getHandle();
		pipelineDescription.subpass = 0;

		defaultPipeline = std::make_unique<RenderPipeline>(application.getDevice(), pipelineDescription);
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
		vkCmdDraw(commandBuffer, 3, 1, 0, 0);
		renderContext.endFrame();
	}
}

