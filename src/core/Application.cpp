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

		initDescriptorSetLayouts();

		stagingManager = std::make_unique<StagingManager>(*device);
		textureBundle = std::make_unique<TextureBundle>(*device, *textureLayout, *stagingManager);
		renderPass = std::make_unique<RenderPass>(*device, device->getSurfaceFormat().format);
		renderContext = std::make_unique<RenderContext>(*device, *window, *renderPass, *mvpLayout);

		textureBundle->add("main_atlas", "data/images/main_atlas.png");
	}

	Application::~Application()
	{
		if (device) {
			device->waitIdle();
		}

		currentView.reset();
		renderContext.reset();
		renderPass.reset();
		textureBundle.reset();
		stagingManager.reset();
		mvpLayout.reset();
		textureLayout.reset();
		device.reset();
		window.reset();
		instance.reset();
	}

	const RenderPass& Application::getRenderPass() const
	{
		return *renderPass;
	}

	const VulkanDevice& Application::getDevice() const
	{
		return *device;
	}

	StagingManager& Application::getStagingManager()
	{
		return *stagingManager;
	}

	const DescriptorSetLayout& Application::getMVPLayout() const
	{
		return *mvpLayout;
	}

	const DescriptorSetLayout& Application::getTextureLayout() const
	{
		return *textureLayout;
	}

	const TextureBundle& Application::getTextureBundle() const
	{
		return *textureBundle;
	}

    Window& Application::getWindow()
    {
		return *window;
    }

	void Application::run()
	{
		currentView = std::make_unique<GameView>(*this);
		mainLoop();
	}

	void Application::onWindowResize(uint32_t newWidth, uint32_t newHeight)
	{
	}

    void Application::initDescriptorSetLayouts()
    {
		VkDescriptorSetLayoutBinding uniformBinding{};
		uniformBinding.binding = 0;
		uniformBinding.descriptorCount = 1;
		uniformBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		uniformBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		mvpLayout = std::make_unique<DescriptorSetLayout>(*device, std::vector<VkDescriptorSetLayoutBinding>{ uniformBinding });

		VkDescriptorSetLayoutBinding samplerBinding = {};
		samplerBinding.binding = 0;
		samplerBinding.descriptorCount = 1;
		samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerBinding.pImmutableSamplers = nullptr;
		samplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		textureLayout = std::make_unique<DescriptorSetLayout>(*device, std::vector<VkDescriptorSetLayoutBinding>{ samplerBinding });
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