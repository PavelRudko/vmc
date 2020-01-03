#include "RenderContext.h"
#include <stdexcept>

namespace vmc
{
	RenderContext::RenderContext(VulkanDevice& device, const Window& window) :
		device(device)
	{
		swapchain = std::make_unique<VulkanSwapchain>(device, window.getSurface(), window.getWidth(), window.getHeight());
		initRenderPass();
		initCommandPool();
		initSwapchainResources();
		initFrameResources();
	}

	RenderContext::~RenderContext()
	{
		if (renderPass != VK_NULL_HANDLE) {
			vkDestroyRenderPass(device.getHandle(), renderPass, nullptr);
		}

		for (const auto& frameResource : frameResources) {
			vkDestroySemaphore(device.getHandle(), frameResource.imageAvailableSemaphore, nullptr);
			vkDestroySemaphore(device.getHandle(), frameResource.renderingFinishedSemaphore, nullptr);
			vkDestroyFence(device.getHandle(), frameResource.fence, nullptr);
		}

		destroySwapchainResources();

		if (commandPool != VK_NULL_HANDLE) {
			vkDestroyCommandPool(device.getHandle(), commandPool, nullptr);
		}

		swapchain.reset();
	}

	VkCommandBuffer RenderContext::startFrame(VkClearColorValue clearColor)
	{
		if (isFrameStarted) {
			throw std::runtime_error("Cannot start a new frame before the previous is ended.");
		}
		
		auto& resource = frameResources[frameResourceIndex];
		auto commandBuffer = commandBuffers[frameResourceIndex];

		vkWaitForFences(device.getHandle(), 1, &resource.fence, VK_TRUE, UINT64_MAX);
		vkResetFences(device.getHandle(), 1, &resource.fence);

		vkResetCommandBuffer(commandBuffer, 0);

		auto result = vkAcquireNextImageKHR(device.getHandle(), swapchain->getHandle(), UINT64_MAX, resource.imageAvailableSemaphore, VK_NULL_HANDLE, &currentImageIndex);
		if (result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR) {
			handleSurfaceChanges();
			vkAcquireNextImageKHR(device.getHandle(), swapchain->getHandle(), UINT64_MAX, resource.imageAvailableSemaphore, VK_NULL_HANDLE, &currentImageIndex);
		}

		beginRecordingCommandBuffer(commandBuffer, framebuffers[currentImageIndex], clearColor);
		isFrameStarted = true;
		return commandBuffer;
	}

	void RenderContext::endFrame()
	{
		if (!isFrameStarted) {
			throw std::runtime_error("Cannot end the frame before it is started.");
		}

		auto& resource = frameResources[frameResourceIndex];
		auto commandBuffer = commandBuffers[frameResourceIndex];

		endRecordingCommandBuffer(commandBuffer);

		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &resource.imageAvailableSemaphore;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &resource.renderingFinishedSemaphore;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		if (vkQueueSubmit(device.getGraphicsQueue(), 1, &submitInfo, resource.fence) != VK_SUCCESS) {
			throw std::runtime_error("Cannot submit command buffer.");
		}

		auto swapchainHandle = swapchain->getHandle();
		VkPresentInfoKHR presentInfo{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &resource.renderingFinishedSemaphore;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &swapchainHandle;
		presentInfo.pImageIndices = &currentImageIndex;

		auto result = vkQueuePresentKHR(device.getPresentQueue(), &presentInfo);
		if (result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR) {
			handleSurfaceChanges();
		}
		else if (result != VK_SUCCESS) {
			throw std::runtime_error("Cannot present to swapchain.");
		}

		frameResourceIndex = (frameResourceIndex + 1) % (uint32_t)frameResources.size();
		isFrameStarted = false;
	}

	void RenderContext::initImageViews()
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

			if (vkCreateImageView(device.getHandle(), &createInfo, nullptr, &imageViews[i]) != VK_SUCCESS) {
				throw std::runtime_error("Cannot create image view.");
			}
		}
	}

	void RenderContext::initRenderPass()
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

		VkRenderPassCreateInfo createInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
		createInfo.attachmentCount = 1;
		createInfo.pAttachments = &colorAttachment;
		createInfo.subpassCount = 1;
		createInfo.pSubpasses = &subpass;
		createInfo.dependencyCount = 1;
		createInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(device.getHandle(), &createInfo, nullptr, &renderPass) != VK_SUCCESS) {
			throw std::runtime_error("Cannot create renderpass.");
		}
	}

	void RenderContext::initFramebuffers()
	{
		auto extent = swapchain->getExtent();
		framebuffers.resize(imageViews.size(), VK_NULL_HANDLE);
		for (uint32_t i = 0; i < framebuffers.size(); i++) {
			VkFramebufferCreateInfo createInfo{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
			createInfo.attachmentCount = 1;
			createInfo.pAttachments = &imageViews[i];
			createInfo.width = extent.width;
			createInfo.height = extent.height;
			createInfo.layers = 1;
			createInfo.renderPass = renderPass;

			if (vkCreateFramebuffer(device.getHandle(), &createInfo, nullptr, &framebuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("Cannot create framebuffer.");
			}
		}
	}

	void RenderContext::initCommandPool()
	{
		VkCommandPoolCreateInfo createInfo{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
		createInfo.queueFamilyIndex = device.getGraphicsQueueFamilyIndex();
		createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		if (vkCreateCommandPool(device.getHandle(), &createInfo, nullptr, &commandPool) != VK_SUCCESS) {
			throw std::runtime_error("Cannot create command pool.");
		}
	}

	void RenderContext::initCommandBuffers()
	{
		commandBuffers.resize(framebuffers.size());

		VkCommandBufferAllocateInfo allocateInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
		allocateInfo.commandPool = commandPool;
		allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocateInfo.commandBufferCount = commandBuffers.size();

		if (vkAllocateCommandBuffers(device.getHandle(), &allocateInfo, commandBuffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("Cannot allocate command buffers.");
		}
	}

	void RenderContext::initFrameResources()
	{
		frameResources.resize(framebuffers.size());

		VkSemaphoreCreateInfo semaphoreCreateInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
		VkFenceCreateInfo fenceCreateInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
		fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (uint32_t i = 0; i < frameResources.size(); i++) {
			if (vkCreateSemaphore(device.getHandle(), &semaphoreCreateInfo, nullptr, &frameResources[i].imageAvailableSemaphore) != VK_SUCCESS ||
				vkCreateSemaphore(device.getHandle(), &semaphoreCreateInfo, nullptr, &frameResources[i].renderingFinishedSemaphore) != VK_SUCCESS) {
				throw std::runtime_error("Cannot create semaphore.");
			}

			if (vkCreateFence(device.getHandle(), &fenceCreateInfo, nullptr, &frameResources[i].fence) != VK_SUCCESS) {
				throw std::runtime_error("Cannot create fence.");
			}
		}
	}

	void RenderContext::handleSurfaceChanges()
	{
		VkSurfaceCapabilitiesKHR properties;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.getPhysicalDevice(), swapchain->getSurface(), &properties);

		auto extent = swapchain->getExtent();
		auto newExtent = properties.currentExtent;

		if (newExtent.width != extent.width || newExtent.height != extent.height) {
			device.waitIdle();
			destroySwapchainResources();
			swapchain = std::make_unique<VulkanSwapchain>(*swapchain, newExtent.width, newExtent.height);
			initSwapchainResources();
		}
	}

	void RenderContext::destroySwapchainResources()
	{
		for (auto framebuffer : framebuffers) {
			vkDestroyFramebuffer(device.getHandle(), framebuffer, nullptr);
		}

		for (auto imageView : imageViews) {
			vkDestroyImageView(device.getHandle(), imageView, nullptr);
		}

		vkFreeCommandBuffers(device.getHandle(), commandPool, commandBuffers.size(), commandBuffers.data());
		commandBuffers.clear();
	}

	void RenderContext::initSwapchainResources()
	{
		initImageViews();
		initFramebuffers();
		initCommandBuffers();
	}

	void RenderContext::beginRecordingCommandBuffer(VkCommandBuffer commandBuffer, VkFramebuffer framebuffer, VkClearColorValue clearColor)
	{
		VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		VkClearValue clearValue;
		clearValue.color = clearColor;

		VkRenderPassBeginInfo renderPassInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = framebuffer;
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = swapchain->getExtent();
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearValue;

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	}

	void RenderContext::endRecordingCommandBuffer(VkCommandBuffer commandBuffer)
	{
		vkCmdEndRenderPass(commandBuffer);
		vkEndCommandBuffer(commandBuffer);
	}
}