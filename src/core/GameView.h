#pragma once

#include "View.h"
#include <rendering/RenderPipeline.h>
#include <vk/VulkanBuffer.h>
#include <rendering/Camera.h>
#include <rendering/MeshBuilder.h>

namespace vmc
{
	class GameView : public View
	{
	public:
		GameView(Application& application);

		virtual ~GameView();

		virtual void update(float timeDelta) override;

		virtual void render(RenderContext& renderContext) override;

	private:
		std::unique_ptr<RenderPipeline> defaultPipeline;
		std::unique_ptr<Mesh> mesh;
		VkDescriptorSet mainAtlasDescriptor;
		Camera camera;
        MeshBuilder meshBuilder;
		bool isCursorLocked = false;

		void initPipeline();
		void initMesh();
		void lockCursor();
		void unlockCursor();
	};
}