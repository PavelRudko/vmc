#pragma once

#include <core/Window.h>
#include <vk/VulkanDevice.h>
#include <vk/VulkanInstance.h>
#include <vk/VulkanSwapchain.h>
#include <memory>

namespace vmc
{
	struct FrameResources
	{
		VkSemaphore renderingFinishedSemaphore;
		VkSemaphore imageAvailableSemaphore;
		VkFence fence;
	};

	class Application
	{
	public:
		Application(uint32_t windowWidth, uint32_t windowHeight);

		Application(const Application&) = delete;

		Application(Application&& other) = delete;

		~Application();

		Application& operator=(const Application&) = delete;

		Application& operator=(Application&&) = delete;

		void run();

		void onWindowResize(uint32_t newWidth, uint32_t newHeight);

	private:
		std::unique_ptr<Window> window;

		std::unique_ptr<VulkanInstance> instance;

		std::unique_ptr<VulkanDevice> device;

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

		void preRecordCommandBuffers();

		void initFrameResources();

		void draw();

		void cleanupSwapchain();

		void recreateSwapchain();
	};
}