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
		renderContext = std::make_unique<RenderContext>(*device, *window);
	}

	Application::~Application()
	{
		if (device) {
			device->waitIdle();
		}

		renderContext.reset();
		device.reset();
		window.reset();
		instance.reset();
	}

	void Application::run()
	{
		while (!window->shouldClose()) {
			window->pollEvents();
			if (window->isFocused()) {
				draw();
			}
		}
	}

	void Application::onWindowResize(uint32_t newWidth, uint32_t newHeight)
	{
	}

	void Application::draw()
	{
		auto commandBuffer = renderContext->startFrame({ 0.8f, 0.9f, 1.0f, 1.0f });
		renderContext->endFrame();
	}
}