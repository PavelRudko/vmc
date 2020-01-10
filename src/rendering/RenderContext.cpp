#include "RenderContext.h"
#include <stdexcept>
#include <glm/glm.hpp>

namespace vmc
{
	RenderContext::RenderContext(VulkanDevice& device, const Window& window, const RenderPass& renderPass, const DescriptorSetLayout& mvpLayout) :
		renderPass(renderPass),
		device(device)
	{
		swapchain = std::make_unique<VulkanSwapchain>(device, device.getSurfaceFormat(), window.getSurface(), window.getWidth(), window.getHeight());
		initCommandPool();
		initSwapchainResources();
		initFrameResources(mvpLayout);
	}

	RenderContext::~RenderContext()
	{
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
		resource.mvpUniform->reset();
		return commandBuffer;
	}

	void RenderContext::endFrame()
	{
		if (!isFrameStarted) {
			throw std::runtime_error("Cannot end the frame before it is started.");
		}

		auto& resource = frameResources[frameResourceIndex];
		auto commandBuffer = commandBuffers[frameResourceIndex];
		resource.mvpUniform->flush();

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

	uint32_t RenderContext::getWidth() const
	{
		return swapchain->getExtent().width;
	}

	uint32_t RenderContext::getHeight() const
	{
		return swapchain->getExtent().height;
	}

	DynamicUniform& RenderContext::getMVPUniform()
	{
		return *frameResources[frameResourceIndex].mvpUniform;
	}

	void RenderContext::initImages()
	{
		auto swapchainImages = swapchain->getImages();

		for (uint32_t i = 0; i < swapchainImages.size(); i++) {
			swapchainImageViews.emplace_back(device, swapchainImages[i], device.getSurfaceFormat().format, VK_IMAGE_ASPECT_COLOR_BIT);

			depthImages.emplace_back(device, getWidth(), getHeight(), device.getDepthFormat(), VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
			depthImageViews.emplace_back(device, depthImages.back(), VK_IMAGE_ASPECT_DEPTH_BIT);
		}
	}

	void RenderContext::initFramebuffers()
	{
		auto extent = swapchain->getExtent();
		framebuffers.resize(swapchainImageViews.size(), VK_NULL_HANDLE);
		for (uint32_t i = 0; i < framebuffers.size(); i++) {
			std::vector<VkImageView> attachments = { swapchainImageViews[i].getHandle(), depthImageViews[i].getHandle() };

			VkFramebufferCreateInfo createInfo{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
			createInfo.attachmentCount = attachments.size();
			createInfo.pAttachments = attachments.data();
			createInfo.width = extent.width;
			createInfo.height = extent.height;
			createInfo.layers = 1;
			createInfo.renderPass = renderPass.getHandle();

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

	void RenderContext::initFrameResources(const DescriptorSetLayout& mvpLayout)
	{
		frameResources.resize(framebuffers.size());
		descriptorPool = std::make_unique<DescriptorPool>(device, 0, framebuffers.size(), 0, framebuffers.size());

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

			frameResources[i].mvpUniform = std::make_unique<DynamicUniform>(device, *descriptorPool, mvpLayout.getHandle(), sizeof(glm::mat4), 256);
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

		vkFreeCommandBuffers(device.getHandle(), commandPool, commandBuffers.size(), commandBuffers.data());
		commandBuffers.clear();
	}

	void RenderContext::initSwapchainResources()
	{
		initImages();
		initFramebuffers();
		initCommandBuffers();
	}

	void RenderContext::beginRecordingCommandBuffer(VkCommandBuffer commandBuffer, VkFramebuffer framebuffer, VkClearColorValue clearColor)
	{
		VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		std::vector< VkClearValue> clearValues(2);
		clearValues[0].color = clearColor;
		clearValues[1].depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo renderPassInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
		renderPassInfo.renderPass = renderPass.getHandle();
		renderPassInfo.framebuffer = framebuffer;
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = swapchain->getExtent();
		renderPassInfo.clearValueCount = clearValues.size();
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		auto extent = swapchain->getExtent();

		VkViewport viewport;
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)extent.width;
		viewport.height = (float)extent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor;
		scissor.offset = { 0, 0 };
		scissor.extent = extent;

		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}

	void RenderContext::endRecordingCommandBuffer(VkCommandBuffer commandBuffer)
	{
		vkCmdEndRenderPass(commandBuffer);
		vkEndCommandBuffer(commandBuffer);
	}
}