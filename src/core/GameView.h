#pragma once

#include "View.h"
#include <rendering/RenderPipeline.h>
#include <vk/VulkanBuffer.h>
#include <rendering/Camera.h>
#include <rendering/Mesh.h>
#include <world/Chunk.h>
#include <world/World.h>

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
        std::unordered_map<glm::ivec2, Mesh> chunkMeshes;
		VkDescriptorSet mainAtlasDescriptor;
		Camera camera;
        World world;
		bool isCursorLocked = false;

		void initPipeline();
        void initChunks();
		void initMeshes();
		void lockCursor();
		void unlockCursor();
	};
}