#pragma once

#include <vk/VulkanDevice.h>
#include <vk/VulkanSwapchain.h>
#include "RenderPass.h"
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
		RenderContext(VulkanDevice& device, const Window& window, const RenderPass& renderPass);

		RenderContext(const RenderContext&) = delete;

		RenderContext(RenderContext&& other) = delete;

		~RenderContext();

		RenderContext& operator=(const RenderContext&) = delete;

		RenderContext& operator=(RenderContext&&) = delete;

		VkCommandBuffer startFrame(VkClearColorValue clearColor);

		void endFrame();

	private:
		VulkanDevice& device;

		std::unique_ptr<VulkanSwapchain> swapchain;

		std::vector<VkImageView> imageViews;

		std::vector<VkFramebuffer> framebuffers;

		const RenderPass& renderPass;

		VkCommandPool commandPool = VK_NULL_HANDLE;

		std::vector<VkCommandBuffer> commandBuffers;

		std::vector<FrameResources> frameResources;

		uint32_t frameResourceIndex = 0;

		void initImageViews();

		void initFramebuffers();

		void initCommandPool();

		void initCommandBuffers();

		void initFrameResources();

		void handleSurfaceChanges();

		void destroySwapchainResources();

		void initSwapchainResources();

		bool isFrameStarted = false;

		uint32_t currentImageIndex = 0;

		void beginRecordingCommandBuffer(VkCommandBuffer commandBuffer, VkFramebuffer framebuffer, VkClearColorValue clearColor);

		void endRecordingCommandBuffer(VkCommandBuffer commandBuffer);
	};
}