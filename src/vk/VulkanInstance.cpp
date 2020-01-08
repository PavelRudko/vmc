#include "VulkanInstance.h"
#include <stdexcept>
#include <algorithm>
#include "common/Log.h"

namespace vmc
{
#ifdef VULKAN_DEBUG
    VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugReportFlagsEXT flags, 
                                                        VkDebugReportObjectTypeEXT type,
                                                        uint64_t object, 
                                                        size_t location, 
                                                        int32_t messageCode,
                                                        const char* layerPrefix, 
                                                        const char* message, 
                                                        void* userData)
    {
        if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
        {
            loge("%s: %s", layerPrefix, message);
        }
        else
        {
            logd("%s: %s", layerPrefix, message);
        }
        return VK_FALSE;
    }
#endif

    std::vector<VkExtensionProperties> getAvailableInstanceExtensions()
    {
        uint32_t extensionsCount;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, nullptr);

        std::vector<VkExtensionProperties> extensions(extensionsCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, extensions.data());
        return extensions;
    }

    std::vector<VkLayerProperties> getAvailableInstanceLayers()
    {
        uint32_t layersCount;
        vkEnumerateInstanceLayerProperties(&layersCount, nullptr);

        std::vector<VkLayerProperties> layers(layersCount);
        vkEnumerateInstanceLayerProperties(&layersCount, layers.data());

        return layers;
    }

    std::vector<VkPhysicalDevice> getInstancePhysicalDevices(VkInstance instance)
    {
        uint32_t devicesCount;
        vkEnumeratePhysicalDevices(instance, &devicesCount, nullptr);

        std::vector<VkPhysicalDevice> devices(devicesCount);
        vkEnumeratePhysicalDevices(instance, &devicesCount, devices.data());

        return devices;
    }

    bool checkExtensions(const std::vector<const char*>& requiredExtensions, const std::vector<VkExtensionProperties>& availableExtensions)
    {
        for (auto requiredExtension : requiredExtensions) {
            if (!std::any_of(availableExtensions.begin(), availableExtensions.end(),
                [requiredExtension](const VkExtensionProperties& availableExtension) {
                return strcmp(availableExtension.extensionName, requiredExtension) == 0;
            }
            )) {
                return false;
            }
        }
        return true;
    }

    bool checkLayers(const std::vector<const char*>& requiredLayers, const std::vector<VkLayerProperties>& availableLayers)
    {
        for (auto requiredLayer : requiredLayers) {
            if (!std::any_of(availableLayers.begin(), availableLayers.end(),
                [requiredLayer](const VkLayerProperties& availableExtension) {
                return strcmp(availableExtension.layerName, requiredLayer) == 0;
            }
            )) {
                return false;
            }
        }
        return true;
    }

	VulkanInstance::VulkanInstance(const std::string& applicationName, 
		                           const std::string& engineName, 
		                           const std::vector<const char*>& requiredExtensions, 
		                           const std::vector<const char*>& requiredLayers)
	{
        if (volkInitialize()) {
            throw::std::runtime_error("Cannot initialize volk.");
        }

        std::vector<const char*> enabledExtensions(requiredExtensions);
        std::vector<const char*> enabledLayers(requiredLayers);

#ifdef VULKAN_DEBUG
        enabledExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
        enabledLayers.push_back("VK_LAYER_KHRONOS_validation");
#endif

        auto availableExtensions = getAvailableInstanceExtensions();
        if (!checkExtensions(enabledExtensions, availableExtensions)) {
            throw::std::runtime_error("Required instance extensions are missing.");
        }

        auto availableLayers = getAvailableInstanceLayers();
        if (!checkLayers(enabledLayers, availableLayers)) {
            throw::std::runtime_error("Required instance layers are missing.");
        }

        VkApplicationInfo appInfo{ VK_STRUCTURE_TYPE_APPLICATION_INFO };
        appInfo.pApplicationName = applicationName.c_str();
        appInfo.pEngineName = engineName.c_str();
        appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);

        VkInstanceCreateInfo instanceInfo{ VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
        instanceInfo.pApplicationInfo = &appInfo;
        instanceInfo.enabledExtensionCount = enabledExtensions.size();
        instanceInfo.ppEnabledExtensionNames = enabledExtensions.data();
        instanceInfo.enabledLayerCount = enabledLayers.size();
        instanceInfo.ppEnabledLayerNames = enabledLayers.data();

        if (vkCreateInstance(&instanceInfo, nullptr, &handle) != VK_SUCCESS) {
            throw::std::runtime_error("Cannot initialize vulkan instance.");
        }

        volkLoadInstance(handle);

#ifdef VULKAN_DEBUG
        VkDebugReportCallbackCreateInfoEXT debugCallbackInfo{ VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT };
        debugCallbackInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
        debugCallbackInfo.pfnCallback = debugCallback;
        if (vkCreateDebugReportCallbackEXT(handle, &debugCallbackInfo, nullptr, &debugReportCallback) != VK_SUCCESS) {
            throw std::runtime_error("Cannot setup vulkan debug callback.");
        }
#endif

        physicalDevices = getInstancePhysicalDevices(handle);
	}

	VulkanInstance::VulkanInstance(VulkanInstance&& other) noexcept :
		handle(other.handle),
#ifdef VULKAN_DEBUG
        debugReportCallback(other.debugReportCallback)
#endif
	{
		other.handle = VK_NULL_HANDLE;
#ifdef VULKAN_DEBUG
        other.debugReportCallback = VK_NULL_HANDLE;
#endif
	}

	VulkanInstance::~VulkanInstance()
	{
#ifdef VULKAN_DEBUG
        if (debugReportCallback != VK_NULL_HANDLE) {
            vkDestroyDebugReportCallbackEXT(handle, debugReportCallback, nullptr);
        }
#endif

		if (handle != VK_NULL_HANDLE) {
			vkDestroyInstance(handle, nullptr);
		}
	}

	VkInstance VulkanInstance::getHandle() const
	{
		return handle;
	}

    const std::vector<VkPhysicalDevice>& VulkanInstance::getPhysicalDevices() const
    {
        return physicalDevices;
    }

    VkPhysicalDevice VulkanInstance::getBestPhysicalDevice() const
    {
        for (auto device : physicalDevices) {
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(device, &properties);
            if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                return device;
            }
        }
        return physicalDevices[0];
    }
}