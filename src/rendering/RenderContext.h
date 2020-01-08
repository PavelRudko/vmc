#pragma once

#include <vk/VulkanDevice.h>
#include <vk/Swapchain.h>
#include "RenderPass.h"
#include <core/Window.h>
#include <vk/DescriptorSetLayout.h>
#include <vk/DynamicUniform.h>
#include <memory>

namespace vmc
{
	struct FrameResources
	{
		VkSemaphore renderingFinishedSemaphore;
		VkSemaphore imageAvailableSemaphore;
		VkFence fence;
		std::unique_ptr<DynamicUniform> mvpUniform;
	};

	class RenderContext
	{
	public:
		RenderContext(VulkanDevice& device, const Window& window, const RenderPass& renderPass, const DescriptorSetLayout& mvpLayout);

		RenderContext(const RenderContext&) = delete;

		RenderContext(RenderContext&& other) = delete;

		~RenderContext();

		RenderContext& operator=(const RenderContext&) = delete;

		RenderContext& operator=(RenderContext&&) = delete;

		VkCommandBuffer startFrame(VkClearColorValue clearColor);

		void endFrame();

		uint32_t getWidth() const;

		uint32_t getHeight() const;

		DynamicUniform& getMVPUniform();

	private:
		VulkanDevice& device;

		std::unique_ptr<VulkanSwapchain> swapchain;

		std::unique_ptr<DescriptorPool> descriptorPool;

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

		void initFrameResources(const DescriptorSetLayout& mvpLayout);

		void handleSurfaceChanges();

		void destroySwapchainResources();

		void initSwapchainResources();

		bool isFrameStarted = false;

		uint32_t currentImageIndex = 0;

		void beginRecordingCommandBuffer(VkCommandBuffer commandBuffer, VkFramebuffer framebuffer, VkClearColorValue clearColor);

		void endRecordingCommandBuffer(VkCommandBuffer commandBuffer);
	};
}