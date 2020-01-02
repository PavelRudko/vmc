#pragma once

#include <vk/VulkanInstance.h>
#include <GLFW/glfw3.h>
#include <string>
#include <vector>

namespace vmc
{
	class Application;

	class Window
	{
	public:
		Window(Application& application, const VulkanInstance& instance, uint32_t width, uint32_t height, const std::string& title);

		Window(const Window&) = delete;

		Window(Window&& other) = delete;

		~Window();

		Window& operator=(const Window&) = delete;

		Window& operator=(Window&&) = delete;

		uint32_t getWidth() const;

		uint32_t getHeight() const;

		VkSurfaceKHR getSurface() const;

		bool shouldClose() const;

		void pollEvents();

	private:
		GLFWwindow* handle = nullptr;

		uint32_t width = 0;

		uint32_t height = 0;

		VkSurfaceKHR surface = VK_NULL_HANDLE;

		Application& application;

		void onResize(uint32_t newWidth, uint32_t newHeight);

		static void resizeCallback(GLFWwindow* window, int width, int height);
	};

	void addWindowInstanceExtensions(std::vector<const char*>& extensions);
}