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

		void initPipeline();
		void initBuffers();
	};
}