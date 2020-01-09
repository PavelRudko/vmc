#pragma once

#include "View.h"
#include <rendering/RenderPipeline.h>
#include <vk/VulkanBuffer.h>
#include <vk/DescriptorPool.h>
#include <vk/DynamicUniform.h>
#include <vk/DescriptorSetLayout.h>
#include <glm/glm.hpp>
#include <vk/VulkanImage.h>

namespace vmc
{
	struct ColorVertex
	{
		glm::vec4 position;
		glm::vec2 uv;
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
		VkDescriptorSet mainAtlasDescriptor;

		void initPipeline();
		void initBuffers();
	};
}