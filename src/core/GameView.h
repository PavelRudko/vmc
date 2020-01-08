#pragma once

#include "View.h"
#include <rendering/RenderPipeline.h>
#include <vk/VulkanBuffer.h>
#include <glm/glm.hpp>

namespace vmc
{
	struct ColorVertex
	{
		glm::vec4 position;
		glm::vec4 color;
	};

	struct UniformData
	{
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 projection;
	};

	class GameView : public View
	{
	public:
		GameView(Application& application);

		virtual ~GameView();

		virtual void update(float timeDelta) override;

		virtual void render(RenderContext& renderContext) override;

	private:
		std::unique_ptr<RenderPipeline> defaultPipeline;
		std::unique_ptr<VulkanBuffer> vertexBuffer;
		std::unique_ptr<VulkanBuffer> indexBuffer;
		std::vector<VulkanBuffer> uniformBuffers;
		VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
		VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
		std::vector<VkDescriptorSet> descriptorSets;
		UniformData uniforms;

		void initDescriptorSetLayout();
		void initDescriptorPool();
		void initPipeline();
		void initBuffers();
	};
}