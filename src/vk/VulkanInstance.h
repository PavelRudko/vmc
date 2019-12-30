#pragma once

#include <volk.h>
#include <vector>
#include <string>

namespace vmc
{
#if defined(DEBUG) || defined(_DEBUG)
#	define VULKAN_DEBUG
#endif

	class VulkanInstance
	{
	public:
		VulkanInstance(const std::string& applicationName, 
			           const std::string& engineName, 
			           const std::vector<const char*>& requiredExtensions, 
			           const std::vector<const char*>& requiredLayers);

		VulkanInstance(const VulkanInstance&) = delete;

		VulkanInstance(VulkanInstance&& other) noexcept;

		~VulkanInstance();

		VulkanInstance& operator=(const VulkanInstance&) = delete;

		VulkanInstance& operator=(VulkanInstance&&) = delete;

		VkInstance getHandle() const;

		const std::vector<VkPhysicalDevice>& getPhysicalDevices() const;

		VkPhysicalDevice getBestPhysicalDevice() const;
	
	private:
		VkInstance handle = VK_NULL_HANDLE;

		std::vector<VkPhysicalDevice> physicalDevices;

#if defined(VULKAN_DEBUG)
		VkDebugReportCallbackEXT debugReportCallback = VK_NULL_HANDLE;
#endif
	};
}