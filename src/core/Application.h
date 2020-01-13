#pragma once

#include <core/Window.h>
#include <vk/VulkanDevice.h>
#include <vk/VulkanInstance.h>
#include <vk/StagingManager.h>
#include <rendering/RenderContext.h>
#include <rendering/TextureBundle.h>
#include <rendering/MeshBuilder.h>
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

		const VulkanDevice& getDevice() const;

		StagingManager& getStagingManager();

		const DescriptorSetLayout& getMVPLayout() const;

		const DescriptorSetLayout& getTextureLayout() const;

		const TextureBundle& getTextureBundle() const;

        const std::vector<Block>& getBlockDescriptions() const;

        const MeshBuilder& getMeshBuilder() const;

		uint32_t getFPS() const;

		Window& getWindow();

		void run();

		void onWindowResize(uint32_t newWidth, uint32_t newHeight);

	private:
		std::unique_ptr<Window> window;

		std::unique_ptr<VulkanInstance> instance;

		std::unique_ptr<VulkanDevice> device;

		std::unique_ptr<DescriptorSetLayout> mvpLayout;

		std::unique_ptr<DescriptorSetLayout> textureLayout;

		std::unique_ptr<TextureBundle> textureBundle;

		std::unique_ptr<RenderPass> renderPass;

		std::unique_ptr<RenderContext> renderContext;

		std::unique_ptr<StagingManager> stagingManager;

        std::vector<Block> blockDescriptions;

        std::unique_ptr<MeshBuilder> meshBuilder;

        std::unique_ptr<View> currentView;

		uint32_t fps;

		void initDescriptorSetLayouts();

		void mainLoop();
	};
}