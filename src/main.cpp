#include <iostream>
#include <stdint.h>
#include "vk/VulkanInstance.h"
#include "vk/VulkanDevice.h"
#include "vk/VulkanSwapchain.h"
#include <GLFW/glfw3.h>

void addGLFWInstanceExtensions(std::vector<const char*>& extensions)
{
    uint32_t count;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&count);
    for (int i = 0; i < count; i++) {
        extensions.push_back(glfwExtensions[i]);
    }
}

std::vector<const char*> getRequiredInstanceExtensions()
{
    std::vector<const char*> extensions;
    addGLFWInstanceExtensions(extensions);
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

void run()
{
    if (glfwInit() == GLFW_FALSE) {
        throw std::runtime_error("Cannot initialize GLFW");
    }

    auto requiredInstanceExtensions = getRequiredInstanceExtensions();
    auto requiredInstanceLayers = getRequiredInstanceLayers();

    vmc::VulkanInstance instance("vmc", "vmc", requiredInstanceExtensions, requiredInstanceLayers);

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    auto window = glfwCreateWindow(640, 480, "vmc", NULL, NULL);

    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(instance.getHandle(), window, NULL, &surface) != VK_SUCCESS) {
        throw std::runtime_error("Cannot initialize window surface.");
    }

    auto requiredDeviceEsctensions = getRequiredDeviceExtensions();
    vmc::VulkanDevice device(instance.getBestPhysicalDevice(), surface, requiredDeviceEsctensions);

    vmc::VulkanSwapchain swapchain(device, surface, 640, 480);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }
}

int main()
{
    try {
        run();
    }
    catch (std::runtime_error e) {
        std::cout << e.what() << std::endl;
        return -1;
    }
    return 0;
}