#pragma once

#include "View.h"
#include <rendering/RenderPipeline.h>
#include <vk/VulkanBuffer.h>
#include <rendering/Camera.h>
#include <rendering/Mesh.h>
#include <world/Chunk.h>
#include <world/World.h>
#include <queue>
#include <deque>

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
		std::deque<glm::ivec2> chunksToLoad;
		VkDescriptorSet mainAtlasDescriptor;
		Camera camera;
        World world;
		bool isCursorLocked = false;
		uint32_t visibleChunkRadius = 4;

		void initPipeline();
        void initChunks();
		void initMeshes();
		void lockCursor();
		void unlockCursor();
		void enqueueChunk(int32_t x, int32_t z);
		void enqueueSurroundingChunks(const glm::vec3& playerPosition);
		void loadNextChunk();
		bool isPendingLoading(const glm::ivec2& coord);
	};
}