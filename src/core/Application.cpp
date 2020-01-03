#include "Application.h"
#include <stdexcept>
#include <common/Log.h>
#include "GameView.h"

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
		renderPass = std::make_unique<RenderPass>(*device, device->getSurfaceFormat().format);
		renderContext = std::make_unique<RenderContext>(*device, *window, *renderPass);
	}

	Application::~Application()
	{
		if (device) {
			device->waitIdle();
		}

		renderContext.reset();
		renderPass.reset();
		device.reset();
		window.reset();
		instance.reset();
	}

	const RenderPass& Application::getRenderPass() const
	{
		return *renderPass;
	}

	void Application::run()
	{
		currentView = std::make_unique<GameView>(*this);
		mainLoop();
	}

	void Application::onWindowResize(uint32_t newWidth, uint32_t newHeight)
	{
	}

	void Application::mainLoop()
	{
		while (!window->shouldClose()) {
			window->pollEvents();

			if (currentView) {
				currentView->update(1.0f / 60.0f);
				if (window->isFocused()) {
					currentView->render(*renderContext);
				}
			}
		}
	}
}