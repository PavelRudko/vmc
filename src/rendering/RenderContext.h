#pragma once

#include <vk/VulkanDevice.h>
#include <vk/VulkanSwapchain.h>
#include <core/Window.h>
#include <memory>

namespace vmc
{
	struct FrameResources
	{
		VkSemaphore renderingFinishedSemaphore;
		VkSemaphore imageAvailableSemaphore;
		VkFence fence;
	};

	class RenderContext
	{
	public:
		RenderContext(VulkanDevice& device, const Window& window);

		RenderContext(const RenderContext&) = delete;

		RenderContext(RenderContext&& other) = delete;

		~RenderContext();

		RenderContext& operator=(const RenderContext&) = delete;

		RenderContext& operator=(RenderContext&&) = delete;

		void draw();

	private:
		VulkanDevice& device;

		std::unique_ptr<VulkanSwapchain> swapchain;

		std::vector<VkImageView> imageViews;

		std::vector<VkFramebuffer> framebuffers;

		VkRenderPass renderPass = VK_NULL_HANDLE;

		VkCommandPool commandPool = VK_NULL_HANDLE;

		std::vector<VkCommandBuffer> commandBuffers;

		std::vector<FrameResources> frameResources;

		uint32_t frameResourceIndex = 0;

		void initImageViews();

		void initRenderPass();

		void initFramebuffers();

		void initCommandPool();

		void initCommandBuffers();

		void initFrameResources();

		void handleSurfaceChanges();

		void destroySwapchainResources();

		void initSwapchainResources();

		void recordCommandBuffer(VkCommandBuffer commandBuffer, VkFramebuffer framebuffer);
	};
}