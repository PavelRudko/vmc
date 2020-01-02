#include "Application.h"
#include <stdexcept>
#include <common/Log.h>

namespace vmc
{
	const char* ApplicationName = "vmc";

	std::vector<const char*> getRequiredInstanceExtensions()
	{
		std::vector<const char*> extensions;
		addWindowInstanceExtensions(extensions);
		return extensions;
	}

	std::vector<const char*> getRequiredInstanceLayers()
	{
		std::vector<const char*> layers;
		return layers;
	}

	std::vector<const char*> getRequiredDeviceExtensions()
	{
		std::vector<const char*> extensions;
		extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		return extensions;
	}

	Application::Application(uint32_t windowWidth, uint32_t windowHeight)
	{
		auto requiredInstanceExtensions = getRequiredInstanceExtensions();
		auto requiredInstanceLayers = getRequiredInstanceLayers();
		auto requiredDeviceExtensions = getRequiredDeviceExtensions();

		instance = std::make_unique<VulkanInstance>(ApplicationName, ApplicationName, requiredInstanceExtensions, requiredInstanceLayers);
		window = std::make_unique<Window>(*this, *instance, windowWidth, windowHeight, ApplicationName);
		device = std::make_unique<VulkanDevice>(instance->getBestPhysicalDevice(), window->getSurface(), requiredDeviceExtensions);
		swapchain = std::make_unique<VulkanSwapchain>(*device, window->getSurface(), windowWidth, windowHeight);

		initImageViews();
		initRenderPass();
		initFramebuffers();
		initCommandPool();
		initCommandBuffers();
		preRecordCommandBuffers();
		initFrameResources();
	}

	Application::~Application()
	{
		if (device) {
			device->waitIdle();
		}

		cleanupSwapchain();
		device.reset();
		window.reset();
		instance.reset();
	}

	void Application::run()
	{
		while (!window->shouldClose()) {
			window->pollEvents();
			draw();
		}
	}

	void Application::onWindowResize(uint32_t newWidth, uint32_t newHeight)
	{
		logd("%u, %u\n", newWidth, newHeight);
	}

	void Application::initImageViews()
	{
		auto swapchainImages = swapchain->getImages();
		imageViews.resize(swapchainImages.size(), VK_NULL_HANDLE);

		for (uint32_t i = 0; i < swapchainImages.size(); i++) {
			VkImageViewCreateInfo createInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
			createInfo.image = swapchainImages[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = swapchain->getFormat();
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.layerCount = 1;
			createInfo.subresourceRange.levelCount = 1;

			if (vkCreateImageView(device->getHandle(), &createInfo, nullptr, &imageViews[i]) != VK_SUCCESS) {
				throw std::runtime_error("Cannot create image view.");
			}
		}
	}

	void Application::initRenderPass()
	{
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = swapchain->getFormat();
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentReference{};
		colorAttachmentReference.attachment = 0;
		colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentReference;

		VkSubpassDependency dependency{};
		dependency.dstSubpass = 0;
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo createInfo{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
		createInfo.attachmentCount = 1;
		createInfo.pAttachments = &colorAttachment;
		createInfo.subpassCount = 1;
		createInfo.pSubpasses = &subpass;
		createInfo.dependencyCount = 1;
		createInfo.pDependencies = &dependency;
		
		if (vkCreateRenderPass(device->getHandle(), &createInfo, nullptr, &renderPass) != VK_SUCCESS) {
			throw std::runtime_error("Cannot create renderpass.");
		}
	}

	void Application::initFramebuffers()
	{
		framebuffers.resize(imageViews.size(), VK_NULL_HANDLE);
		for (uint32_t i = 0; i < framebuffers.size(); i++) {
			VkFramebufferCreateInfo createInfo{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
			createInfo.attachmentCount = 1;
			createInfo.pAttachments = &imageViews[i];
			createInfo.width = window->getWidth();
			createInfo.height = window->getHeight();
			createInfo.layers = 1;
			createInfo.renderPass = renderPass;

			if (vkCreateFramebuffer(device->getHandle(), &createInfo, nullptr, &framebuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("Cannot create framebuffer.");
			}
		}
	}

	void Application::initCommandPool()
	{
		VkCommandPoolCreateInfo createInfo{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
		createInfo.queueFamilyIndex = device->getGraphicsQueueFamilyIndex();

		if (vkCreateCommandPool(device->getHandle(), &createInfo, nullptr, &commandPool) != VK_SUCCESS) {
			throw std::runtime_error("Cannot create command pool.");
		}
	}

	void Application::initCommandBuffers()
	{
		commandBuffers.resize(framebuffers.size());

		VkCommandBufferAllocateInfo allocateInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
		allocateInfo.commandPool = commandPool;
		allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocateInfo.commandBufferCount = commandBuffers.size();

		if (vkAllocateCommandBuffers(device->getHandle(), &allocateInfo, commandBuffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("Cannot allocate command buffers.");
		}
	}

	void Application::preRecordCommandBuffers()
	{
		for (uint32_t i = 0; i < commandBuffers.size(); i++) {
			auto commandBuffer = commandBuffers[i];

			VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
			vkBeginCommandBuffer(commandBuffer, &beginInfo);

			VkClearValue clearColor = { 0.8f, 0.9f, 1.0f, 1.0f };

			VkRenderPassBeginInfo renderPassInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
			renderPassInfo.renderPass = renderPass;
			renderPassInfo.framebuffer = framebuffers[i];
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = { window->getWidth(), window->getHeight() };
			renderPassInfo.clearValueCount = 1;
			renderPassInfo.pClearValues = &clearColor;

			vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdEndRenderPass(commandBuffer);
			vkEndCommandBuffer(commandBuffer);
		}
	}

	void Application::initFrameResources()
	{
		frameResources.resize(framebuffers.size());

		VkSemaphoreCreateInfo semaphoreCreateInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
		VkFenceCreateInfo fenceCreateInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
		fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (uint32_t i = 0; i < frameResources.size(); i++) {
			if (vkCreateSemaphore(device->getHandle(), &semaphoreCreateInfo, nullptr, &frameResources[i].imageAvailableSemaphore) != VK_SUCCESS ||
				vkCreateSemaphore(device->getHandle(), &semaphoreCreateInfo, nullptr, &frameResources[i].renderingFinishedSemaphore) != VK_SUCCESS) {
				throw std::runtime_error("Cannot create semaphore.");
			}

			if (vkCreateFence(device->getHandle(), &fenceCreateInfo, nullptr, &frameResources[i].fence) != VK_SUCCESS) {
				throw std::runtime_error("Cannot create fence.");
			}
		}
	}

	void Application::draw()
	{
		auto& resource = frameResources[frameResourceIndex];
		frameResourceIndex = (frameResourceIndex + 1) % (uint32_t)frameResources.size();

		vkWaitForFences(device->getHandle(), 1, &resource.fence, VK_TRUE, UINT64_MAX);
		vkResetFences(device->getHandle(), 1, &resource.fence);

		uint32_t imageIndex;
		vkAcquireNextImageKHR(device->getHandle(), swapchain->getHandle(), UINT64_MAX, resource.imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &resource.imageAvailableSemaphore;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &resource.renderingFinishedSemaphore;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

		if (vkQueueSubmit(device->getGraphicsQueue(), 1, &submitInfo, resource.fence) != VK_SUCCESS) {
			throw std::runtime_error("Cannot submit command buffer.");
		}

		auto swapchainHandle = swapchain->getHandle();
		VkPresentInfoKHR presentInfo{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &resource.renderingFinishedSemaphore;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &swapchainHandle;
		presentInfo.pImageIndices = &imageIndex;
		if (vkQueuePresentKHR(device->getPresentQueue(), &presentInfo) != VK_SUCCESS) {
			throw std::runtime_error("Cannot present to swapchain.");
		}
	}
	void Application::cleanupSwapchain()
	{
		for (const auto& frameResource : frameResources) {
			vkDestroySemaphore(device->getHandle(), frameResource.imageAvailableSemaphore, nullptr);
			vkDestroySemaphore(device->getHandle(), frameResource.renderingFinishedSemaphore, nullptr);
			vkDestroyFence(device->getHandle(), frameResource.fence, nullptr);
		}

		if (commandPool != VK_NULL_HANDLE) {
			vkDestroyCommandPool(device->getHandle(), commandPool, nullptr);
		}

		for (auto framebuffer : framebuffers) {
			vkDestroyFramebuffer(device->getHandle(), framebuffer, nullptr);
		}

		if (renderPass != VK_NULL_HANDLE) {
			vkDestroyRenderPass(device->getHandle(), renderPass, nullptr);
		}

		for (auto imageView : imageViews) {
			vkDestroyImageView(device->getHandle(), imageView, nullptr);
		}

		swapchain.reset();
	}

	void Application::recreateSwapchain()
	{
		device->waitIdle();
		cleanupSwapchain();
	}
}