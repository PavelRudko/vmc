#pragma once

#include <core/Window.h>
#include <vk/VulkanDevice.h>
#include <vk/VulkanInstance.h>
#include <rendering/RenderContext.h>
#include <memory>
#include "View.h"

namespace vmc
{
	class Application
	{
	public:
		Application(uint32_t windowWidth, uint32_t windowHeight);

		Application(const Application&) = delete;

		Application(Application&& other) = delete;

		~Application();

		Application& operator=(const Application&) = delete;

		Application& operator=(Application&&) = delete;

		const RenderPass& getRenderPass() const;

		void run();

		void onWindowResize(uint32_t newWidth, uint32_t newHeight);

	private:
		std::unique_ptr<Window> window;

		std::unique_ptr<VulkanInstance> instance;

		std::unique_ptr<VulkanDevice> device;

		std::unique_ptr<RenderPass> renderPass;

		std::unique_ptr<RenderContext> renderContext;

		std::unique_ptr<View> currentView;

		void mainLoop();
	};
}