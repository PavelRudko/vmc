#include "VulkanDevice.h"
#include <stdexcept>
#include <set>

namespace vmc
{
	std::vector<VkSurfaceFormatKHR> getFormats(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
	{
		uint32_t count;
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &count, nullptr);

		std::vector<VkSurfaceFormatKHR> formats(count);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &count, formats.data());
		return formats;
	}

	VkSurfaceFormatKHR chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		for (const auto& surfaceFormat : availableFormats) {
			if (surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR && surfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM) {
				return surfaceFormat;
			}
		}

		return availableFormats[0];
	}

	uint32_t findPresentQueueFamilyIndex(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
	{
		uint32_t queueFamiliesCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamiliesCount, nullptr);
		for (uint32_t i = 0; i < queueFamiliesCount; i++) {
			VkBool32 isPresentSupported = VK_FALSE;
			vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &isPresentSupported);
			if (isPresentSupported == VK_TRUE) {
				return i;
			}
		}
		throw std::runtime_error("Cannot find present queue family.");
	}

	uint32_t findQueueFamilyIndex(const std::vector<VkQueueFamilyProperties> queueFamilies, VkQueueFlags requiredFlags, const std::set<uint32_t>& usedQueueFamilyIndices)
	{
		for (uint32_t i = 0; i < queueFamilies.size(); i++) {
			const auto& properties = queueFamilies[i];
			bool isAlreadyUsed = usedQueueFamilyIndices.count(i) > 0;
			if ((properties.queueFlags & requiredFlags) && !isAlreadyUsed) {
				return i;
			}
		}

		for (uint32_t i = 0; i < queueFamilies.size(); i++) {
			if (queueFamilies[i].queueFlags & requiredFlags) {
				return i;
			}
		}

		throw std::runtime_error("Cannot find requested queue family.");
	}

	VulkanDevice::VulkanDevice(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, const std::vector<const char*> requiredDeviceExtensions) :
		physicalDevice(physicalDevice)
	{
		uint32_t queueFamiliesCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamiliesCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamiliesCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamiliesCount, queueFamilies.data());

		std::set<uint32_t> usedQueueFamilyIndices;
		presentQueueFamilyIndex = findPresentQueueFamilyIndex(physicalDevice, surface);
		usedQueueFamilyIndices.insert(presentQueueFamilyIndex);

		graphicsQueueFamilyIndex = findQueueFamilyIndex(queueFamilies, VK_QUEUE_GRAPHICS_BIT, usedQueueFamilyIndices);
		usedQueueFamilyIndices.insert(graphicsQueueFamilyIndex);

		transferQueueFamilyIndex = findQueueFamilyIndex(queueFamilies, VK_QUEUE_TRANSFER_BIT, usedQueueFamilyIndices);
		usedQueueFamilyIndices.insert(transferQueueFamilyIndex);

		computeQueueFamilyIndex = findQueueFamilyIndex(queueFamilies, VK_QUEUE_COMPUTE_BIT, usedQueueFamilyIndices);
		usedQueueFamilyIndices.insert(computeQueueFamilyIndex);

		float queuePriority = 1.0f;
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		for (auto familyIndex : usedQueueFamilyIndices) {
			VkDeviceQueueCreateInfo queueCreateInfo{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfo.queueFamilyIndex = familyIndex;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures requiredFeatures{};

		VkDeviceCreateInfo deviceInfo{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
		deviceInfo.enabledExtensionCount = requiredDeviceExtensions.size();
		deviceInfo.ppEnabledExtensionNames = requiredDeviceExtensions.data();
		deviceInfo.queueCreateInfoCount = queueCreateInfos.size();
		deviceInfo.pQueueCreateInfos = queueCreateInfos.data();
		deviceInfo.pEnabledFeatures = &requiredFeatures;

		if (vkCreateDevice(physicalDevice, &deviceInfo, nullptr, &handle) != VK_SUCCESS) {
			throw std::runtime_error("Cannot initialize vulkan device.");
		}

		vkGetDeviceQueue(handle, presentQueueFamilyIndex, 0, &presentQueue);
		vkGetDeviceQueue(handle, graphicsQueueFamilyIndex, 0, &graphicsQueue);
		vkGetDeviceQueue(handle, computeQueueFamilyIndex, 0, &computeQueue);
		vkGetDeviceQueue(handle, transferQueueFamilyIndex, 0, &transferQueue);

		auto availableFormats = getFormats(physicalDevice, surface);
		surfaceFormat = chooseSurfaceFormat(availableFormats);
	}

	VulkanDevice::VulkanDevice(VulkanDevice&& other) noexcept :
		physicalDevice(other.physicalDevice),
		handle(other.handle),
		presentQueue(other.presentQueue),
		graphicsQueue(other.graphicsQueue),
		computeQueue(other.computeQueue),
		transferQueue(other.transferQueue),
		presentQueueFamilyIndex(other.presentQueueFamilyIndex),
		graphicsQueueFamilyIndex(other.graphicsQueueFamilyIndex),
		transferQueueFamilyIndex(other.transferQueueFamilyIndex),
		computeQueueFamilyIndex(other.computeQueueFamilyIndex)
	{
		other.handle = VK_NULL_HANDLE;
	}

	VulkanDevice::~VulkanDevice()
	{
		if (handle != VK_NULL_HANDLE) {
			waitIdle();
			vkDestroyDevice(handle, nullptr);
		}
	}

	VkQueue VulkanDevice::getPresentQueue() const
	{
		return presentQueue;
	}

	VkQueue VulkanDevice::getGraphicsQueue() const
	{
		return graphicsQueue;
	}

	VkQueue VulkanDevice::getTransferQueue() const
	{
		return transferQueue;
	}

	VkQueue VulkanDevice::getComputeQueue() const
	{
		return computeQueue;
	}

	uint32_t VulkanDevice::getPresentQueueFamilyIndex() const
	{
		return presentQueueFamilyIndex;
	}

	uint32_t VulkanDevice::getGraphicsQueueFamilyIndex() const
	{
		return graphicsQueueFamilyIndex;
	}

	uint32_t VulkanDevice::getComputeQueueFamilyIndex() const
	{
		return computeQueueFamilyIndex;
	}

	uint32_t VulkanDevice::getTransferQueueFamilyIndex() const
	{
		return transferQueueFamilyIndex;
	}

	VkDevice VulkanDevice::getHandle() const
	{
		return handle;
	}

	VkPhysicalDevice VulkanDevice::getPhysicalDevice() const
	{
		return physicalDevice;
	}

	VkSurfaceFormatKHR VulkanDevice::getSurfaceFormat() const
	{
		return surfaceFormat;
	}

	void VulkanDevice::waitIdle() const
	{
		vkDeviceWaitIdle(handle);
	}
}