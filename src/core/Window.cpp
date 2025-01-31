#include "Window.h"
#include <stdexcept>
#include "Application.h"

namespace vmc
{
	void ensureGLFWIsInitialized()
	{
		static bool isInitialized = false;
		if (!isInitialized) {
			if (glfwInit() == GLFW_FALSE) {
				throw std::runtime_error("Cannot initialize GLFW");
			}
			isInitialized = true;
		}
	}

	Window::Window(Application& application, const VulkanInstance& instance, uint32_t width, uint32_t height, const std::string& title) :
		application(application),
		width(width),
		height(height),
		instance(instance)
	{
		ensureGLFWIsInitialized();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		handle = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
		glfwSetWindowSizeLimits(handle, 320, 240, GLFW_DONT_CARE, GLFW_DONT_CARE);

		if (glfwCreateWindowSurface(instance.getHandle(), handle, NULL, &surface) != VK_SUCCESS) {
			throw std::runtime_error("Cannot initialize window surface.");
		}

		glfwSetWindowUserPointer(handle, this);
		glfwSetWindowSizeCallback(handle, resizeCallback);
		glfwSetWindowFocusCallback(handle, focusCallback);
	}

	Window::~Window()
	{
		if (handle) {
			vkDestroySurfaceKHR(instance.getHandle(), surface, nullptr);
			glfwDestroyWindow(handle);
			glfwTerminate();
		}
	}

	uint32_t Window::getWidth() const
	{
		return width;
	}

	uint32_t Window::getHeight() const
	{
		return height;
	}

	VkSurfaceKHR Window::getSurface() const
	{
		return surface;
	}

	bool Window::shouldClose() const
	{
		return glfwWindowShouldClose(handle);
	}

	bool Window::isFocused() const
	{
		return focused;
	}

	void Window::pollEvents()
	{
		glfwPollEvents();
	}

    glm::uvec2 Window::getMousePos() const
    {
		double x, y;
		glfwGetCursorPos(handle, &x, &y);
		return glm::uvec2(x, y);
    }

    void Window::setMousePos(uint32_t x, uint32_t y) const
    {
		glfwSetCursorPos(handle, x, y);
    }

    void Window::setCursorVisibility(bool isVisible)
    {
		int mode = isVisible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED;
		glfwSetInputMode(handle, GLFW_CURSOR, mode);
    }

    bool Window::isKeyPressed(int key)
    {
		return glfwGetKey(handle, key) == GLFW_PRESS;
    }

	bool Window::isMouseButtonPressed(int button)
	{
		return glfwGetMouseButton(handle, button) == GLFW_PRESS;
	}

	void Window::onResize(uint32_t newWidth, uint32_t newHeight)
	{
		width = newWidth;
		height = newHeight;
		application.onWindowResize(newWidth, newHeight);
	}

	void Window::onFocus(bool focused)
	{
		this->focused = focused;
	}

	void Window::resizeCallback(GLFWwindow* windowHandle, int width, int height)
	{
		if (auto window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(windowHandle)))
		{
			window->onResize(width, height);
		}
	}

	void Window::focusCallback(GLFWwindow* windowHandle, int focused)
	{
		if (auto window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(windowHandle)))
		{
			window->onFocus(focused);
		}
	}

	void addWindowInstanceExtensions(std::vector<const char*>& extensions)
	{
		ensureGLFWIsInitialized();
		uint32_t count;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&count);
		for (int i = 0; i < count; i++) {
			extensions.push_back(glfwExtensions[i]);
		}
	}
}