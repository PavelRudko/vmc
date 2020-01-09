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
		std::unique_ptr<VulkanImage> image;
		std::unique_ptr<VulkanImageView> imageView;
		std::unique_ptr<DescriptorPool> imageDescriptorPool;
		std::unique_ptr<DescriptorSetLayout> samplerLayout;
		VkDescriptorSet imageDescriptor;
		VkSampler sampler = VK_NULL_HANDLE;

		void initPipeline();
		void initBuffers();
		void initTexture();
	};
}