#include "GameView.h"
#include <common/Utils.h>
#include <core/Application.h>
#include <vk/ShaderModule.h>
#include <glm/gtc/matrix_transform.hpp>
#include <common/Log.h>

namespace vmc
{
	GameView::GameView(Application& application) :
		View(application),
        meshBuilder(application.getDevice())
	{
		initPipeline();
		initMesh();
		mainAtlasDescriptor = application.getTextureBundle().getDescriptor("main_atlas");
		camera.setPosition({ 0, 0, 2.0f });
		lockCursor();
	}

	GameView::~GameView()
	{
	}

	void GameView::update(float timeDelta)
	{
		auto& window = application.getWindow();

		uint32_t windowCenterX = window.getWidth() / 2;
		uint32_t windowCenterY = window.getHeight() / 2;

		if (!isCursorLocked && window.isMouseButtonPressed(GLFW_MOUSE_BUTTON_1)) {
			lockCursor();
		}

		if (window.isKeyPressed(GLFW_KEY_ESCAPE)) {
			unlockCursor();
		}

		if (isCursorLocked) {
			auto mousePos = window.getMousePos();

			float deltaX = (float)mousePos.x - windowCenterX;
			float deltaY = (float)mousePos.y - windowCenterY;

			camera.addYaw(-deltaX * 0.002f);
			camera.addPitch(-deltaY * 0.002f);
			window.setMousePos(windowCenterX, windowCenterY);
		}

		float speedForward = 0.0f;
		float speedSide = 0.0f;

		if (window.isKeyPressed(GLFW_KEY_W)) {
			speedForward += 1.0f;
		}
		if (window.isKeyPressed(GLFW_KEY_S)) {
			speedForward -= 1.0f;
		}
		if (window.isKeyPressed(GLFW_KEY_A)) {
			speedSide -= 1.0f;
		}
		if (window.isKeyPressed(GLFW_KEY_D)) {
			speedSide += 1.0f;
		}

		camera.moveForward(speedForward * 3.0f * timeDelta);
		camera.moveSide(speedSide * 3.0f * timeDelta);
	}

	void GameView::render(RenderContext& renderContext)
	{
		auto& uniform = renderContext.getMVPUniform();
		auto uniformDescriptorSet = uniform.getDescriptorSet();

		auto viewMatrix = camera.getViewMatrix();
		auto projectionMatrix = glm::perspective(glm::radians(55.0f), (float)renderContext.getWidth() / renderContext.getHeight(), 0.01f, 1000.0f);
		projectionMatrix[1][1] *= -1;

		auto commandBuffer = renderContext.startFrame({ 0.8f, 0.9f, 1.0f, 1.0f });

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, defaultPipeline->getHandle());
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, defaultPipeline->getLayout(), 1, 1, &mainAtlasDescriptor, 0, nullptr);

		for (uint32_t i = 0; i < 2; i++) {
			auto modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(i == 0 ? -1 : 1, 0, 0));
			auto mvp = projectionMatrix * viewMatrix * modelMatrix;

			uint32_t uniformOffset = uniform.pushData(&mvp);
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, defaultPipeline->getLayout(), 0, 1, &uniformDescriptorSet, 1, &uniformOffset);

			VkBuffer vertexBufferHandle = mesh->getVertexBuffer().getHandle();
			VkDeviceSize offset = 0;
			vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBufferHandle, &offset);

			vkCmdBindIndexBuffer(commandBuffer, mesh->getIndexBuffer().getHandle(), 0, VK_INDEX_TYPE_UINT32);

			vkCmdDrawIndexed(commandBuffer, mesh->getIndicesCount(), 1, 0, 0, 0);
		}

		renderContext.endFrame();
	}

	void GameView::initPipeline()
	{
		auto vertexShaderData = readBinaryFile("data/shaders/default.vert.spv");
		auto fragmentShaderData = readBinaryFile("data/shaders/default.frag.spv");

		VulkanShaderModule vertexShader(application.getDevice(), vertexShaderData, VK_SHADER_STAGE_VERTEX_BIT);
		VulkanShaderModule fragmentShader(application.getDevice(), fragmentShaderData, VK_SHADER_STAGE_FRAGMENT_BIT);

		VkVertexInputBindingDescription colorVertexBinding{};
		colorVertexBinding.binding = 0;
		colorVertexBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		colorVertexBinding.stride = sizeof(BlockVertex);

		VkVertexInputAttributeDescription positionAttribute{};
		positionAttribute.binding = 0;
		positionAttribute.location = 0;
		positionAttribute.format = VK_FORMAT_R32G32B32A32_SFLOAT;
		positionAttribute.offset = offsetof(BlockVertex, position);

		VkVertexInputAttributeDescription colorAttribute{};
		colorAttribute.binding = 0;
		colorAttribute.location = 1;
		colorAttribute.format = VK_FORMAT_R32G32_SFLOAT;
		colorAttribute.offset = offsetof(BlockVertex, uv);

		RenderPipelineDescription pipelineDescription;
		pipelineDescription.renderPass = application.getRenderPass().getHandle();
		pipelineDescription.subpass = 0;

		pipelineDescription.shaderModules.push_back(std::move(vertexShader));
		pipelineDescription.shaderModules.push_back(std::move(fragmentShader));
		
		pipelineDescription.vertexBindings.push_back(colorVertexBinding);
		pipelineDescription.vertexAttributes.push_back(positionAttribute);
		pipelineDescription.vertexAttributes.push_back(colorAttribute);

		pipelineDescription.descriptorSetLayouts.push_back(application.getMVPLayout().getHandle());
		pipelineDescription.descriptorSetLayouts.push_back(application.getTextureLayout().getHandle());

		defaultPipeline = std::make_unique<RenderPipeline>(application.getDevice(), pipelineDescription);
	}

	void GameView::initMesh()
	{
        auto& stagingManager = application.getStagingManager();
		stagingManager.start();
        mesh = meshBuilder.buildBlockMesh(stagingManager, 1);
		stagingManager.flush();
	}

	void GameView::lockCursor()
	{
		auto& window = application.getWindow();
		uint32_t windowCenterX = window.getWidth() / 2;
		uint32_t windowCenterY = window.getHeight() / 2;
		window.setMousePos(windowCenterX, windowCenterY);
		window.setCursorVisibility(false);

		isCursorLocked = true;
	}

	void GameView::unlockCursor()
	{
		auto& window = application.getWindow();
		window.setCursorVisibility(true);

		isCursorLocked = false;
	}
}

