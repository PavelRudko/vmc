#pragma once

#include "View.h"
#include <rendering/RenderPipeline.h>
#include <vk/VulkanBuffer.h>
#include <rendering/Camera.h>

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
		Camera camera;
		bool isCursorLocked = false;

		void initPipeline();
		void initBuffers();
		void lockCursor();
		void unlockCursor();
	};
}