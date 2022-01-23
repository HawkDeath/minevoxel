#include "Device.h"
#include "Log.h"

#include <unordered_set>
#include <set>

namespace mv {

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {
        UNUSE(messageSeverity);
        UNUSE(messageType);
        UNUSE(pUserData);
		ELOG("validation layer: {}", pCallbackData->pMessage);

		return VK_FALSE;
	}

	VkResult CreateDebugUtilsMessengerEXT(
		VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pDebugMessenger) {
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
			instance,
			"vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr) {
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void DestroyDebugUtilsMessengerEXT(
		VkInstance instance,
		VkDebugUtilsMessengerEXT debugMessenger,
		const VkAllocationCallbacks* pAllocator) {
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
			instance,
			"vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr) {
			func(instance, debugMessenger, pAllocator);
		}
	}

	Device::Device(Window& window) : mWindow{ window }
	{
		createInstance();
		setupDebugMsg();
		createSurface();
		pickPhysicalDevice();
		createLogicalDevice();
		createCommandPool();
	}

	Device::~Device()
	{
		vkDestroyCommandPool(mDevice, commandPool, CUSTOM_ALLOCATOR);
		vkDestroyDevice(mDevice, CUSTOM_ALLOCATOR);

		if (enableValidationLayers) {
			DestroyDebugUtilsMessengerEXT(instance, debugMessenger, CUSTOM_ALLOCATOR);
		}

		vkDestroySurfaceKHR(instance, surface, CUSTOM_ALLOCATOR);
		vkDestroyInstance(instance, CUSTOM_ALLOCATOR);
	}

	void Device::createInstance()
	{
		if (enableValidationLayers && !checkValidationLayerSupport()) {
			RT_THROW("validation layers requested, but not available");
		}

		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "app";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "engine_app";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_1;

		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		auto extensions = getRequiredExtensions();
		createInfo.enabledExtensionCount = static_cast<std::uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
		if (enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<std::uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();

			populateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
		}
		else {
			createInfo.enabledLayerCount = 0;
			createInfo.pNext = nullptr;
		}

		VK_TEST(vkCreateInstance(&createInfo, CUSTOM_ALLOCATOR, &instance), "Failed to create instance")

        checkRequiredInstanceExtentions();
	}

	void Device::setupDebugMsg()
	{
        if (!enableValidationLayers) return;

        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        populateDebugMessengerCreateInfo(createInfo);
        VK_TEST(CreateDebugUtilsMessengerEXT(instance, &createInfo, CUSTOM_ALLOCATOR, &debugMessenger), "Failed to setup debug messanger")
	}

	void Device::createSurface()
	{
		mWindow.createSurface(instance, &surface);
	}

	void Device::pickPhysicalDevice()
	{
		std::uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
		if (deviceCount == 0) {
			RT_THROW("Failed to find GPUs with Vulkan support");
		}

		LOG("Device count {}", deviceCount);
		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
		
		for (auto& gpu : devices) {
			VkPhysicalDeviceProperties prop;
			vkGetPhysicalDeviceProperties(gpu, &prop);

			LOG("GPU {}, VulkanAPI {}.{}.{}", prop.deviceName, VK_VERSION_MAJOR(prop.apiVersion), VK_VERSION_MINOR(prop.apiVersion), VK_VERSION_PATCH(prop.apiVersion));
		}

		for (auto& gpu : devices) {
			VkPhysicalDeviceProperties prop;
			vkGetPhysicalDeviceProperties(gpu, &prop);
			if (device_helper::isDeviceSuitable(gpu, &surface, deviceExtensions)) {
				if (prop.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) { // prefered GPU
					physicalDevice = gpu;
					break;
				}
				if (prop.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
					physicalDevice = gpu;
				}
			}
		}
		
		if (physicalDevice == VK_NULL_HANDLE) {
			RT_THROW("failed to find suitable GPU");
		}
		vkGetPhysicalDeviceProperties(physicalDevice, &properties);
		LOG("Choosen {} GPU", properties.deviceName);
	}

	void Device::createLogicalDevice()
	{
		QueueFamilyIndices indices = device_helper::findQueueFamilies(physicalDevice, &surface);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<std::uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		float queuePrio = 1.0f;

		for (std::uint32_t queueFamily : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo queueCreateInfo = {};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePrio;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures = {};
		deviceFeatures.samplerAnisotropy = VK_TRUE;

		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

		createInfo.queueCreateInfoCount = static_cast<std::uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();

		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount = static_cast<std::uint32_t>(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();

		// TODO validation layer
		if (enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<std::uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else {
			createInfo.enabledLayerCount = 0;
		}

		VK_TEST(vkCreateDevice(physicalDevice, &createInfo, CUSTOM_ALLOCATOR, &mDevice), "Failed to create logical device")

		vkGetDeviceQueue(mDevice, indices.graphicsFamily.value(), 0, &graphicsQueue);
		vkGetDeviceQueue(mDevice, indices.presentFamily.value(), 0, &presentQueue);
	}

	void Device::createCommandPool()
	{
		QueueFamilyIndices queueFamilyIndices = device_helper::findQueueFamilies(physicalDevice, &surface);

		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
		poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		VK_TEST(vkCreateCommandPool(mDevice, &poolInfo, CUSTOM_ALLOCATOR, &commandPool), "Failed to create command pool")
		
	}

	std::vector<const char*> Device::getRequiredExtensions()
	{
		std::uint32_t glfwExtensionsCount = 0;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionsCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionsCount);

		// TODO: add validation extension VK_EXT_DEBUG_UTILS_EXTENSION_NAME
		if (enableValidationLayers) {
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		return extensions;
	}

	void Device::checkRequiredInstanceExtentions()
	{
		std::uint32_t countExtension = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &countExtension, nullptr);
		std::vector<VkExtensionProperties> extensions(countExtension);
		vkEnumerateInstanceExtensionProperties(nullptr, &countExtension, extensions.data());

		LOG("Vulkan extensions");
		std::unordered_set<std::string> available;
		for (const auto& extension : extensions) {
			LOG("\t{}", extension.extensionName);
			available.insert(extension.extensionName);
		}

		LOG("Required extensions: ");
		auto requiredExtensions = getRequiredExtensions();
		for (const auto& required : requiredExtensions) {
			LOG("\t{}", required);
			if (available.find(required) == available.end()) {
				RT_THROW("missing required extension");
			}
		}
	}

	void Device::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
	{
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
		createInfo.pUserData = nullptr;  // Optional
	}

	bool Device::checkValidationLayerSupport()
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName : validationLayers) {
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers) {
				if (strcmp(layerName, layerProperties.layerName) == 0) {
					layerFound = true;
					break;
				}
			}

			if (!layerFound) {
				return false;
			}
		}

		return true;
	}

	std::uint32_t Device::findMemoryType(std::uint32_t typeFilter, VkMemoryPropertyFlags properties) {
		VkPhysicalDeviceMemoryProperties memProp;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProp);
		for (std::uint32_t i = 0; i < memProp.memoryTypeCount; i++)
		{
			if ((typeFilter & (1 << i)) && (memProp.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}
		RT_THROW("Failed to find suitable memory type");
	}

	VkFormat Device::findSupportedFormat(const std::vector<VkFormat>& formats, VkImageTiling tiling, VkFormatFeatureFlags features) {
		for (VkFormat format : formats) {
			VkFormatProperties prop;
			vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &prop);

			if (tiling == VK_IMAGE_TILING_LINEAR && (prop.linearTilingFeatures & features) == features) {
				return format;
			}
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (prop.optimalTilingFeatures & features) == features) {
				return format;
			}
		}
		RT_THROW("Failed to find supported format");
	}

	void Device::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VK_TEST(vkCreateBuffer(mDevice, &bufferInfo, CUSTOM_ALLOCATOR, &buffer),"Failed to create buffer")
		
		VkMemoryRequirements memReq;
		vkGetBufferMemoryRequirements(mDevice, buffer, &memReq);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memReq.size;
		allocInfo.memoryTypeIndex = findMemoryType(memReq.memoryTypeBits, properties);

		VK_TEST(vkAllocateMemory(mDevice, &allocInfo, CUSTOM_ALLOCATOR, &bufferMemory), "Failed to allocate buffer memory")
		

		// worst case scenario; each buffer (index, vertex, uniform) needs separeate memoryBuffer and buffer
		VK_TEST(vkBindBufferMemory(mDevice, buffer, bufferMemory, 0), "Failed to bind buffer memory")
		
	}
	
	void Device::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		VkBufferCopy copyRegion = {};
		copyRegion.size = size;
		copyRegion.srcOffset = 0;
		copyRegion.dstOffset = 0;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
		endSingleTimeCommands(commandBuffer);
	}
	
	void Device::copyBufferToImage(VkBuffer buffer, VkImage image, std::uint32_t width, std::uint32_t height, std::uint32_t layerCount) {
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		VkBufferImageCopy region = {};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = layerCount;

		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = { width, height, 1 };

		vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
		endSingleTimeCommands(commandBuffer);
	}
	
	void Device::createImageWithInfo(const VkImageCreateInfo& imageInfo, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {

		VK_TEST(vkCreateImage(mDevice, &imageInfo, CUSTOM_ALLOCATOR, &image), "Failed to create image")

		VkMemoryRequirements memReq;
		vkGetImageMemoryRequirements(mDevice, image, &memReq);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memReq.size;
		allocInfo.memoryTypeIndex = findMemoryType(memReq.memoryTypeBits, properties);

		VK_TEST(vkAllocateMemory(mDevice, &allocInfo, CUSTOM_ALLOCATOR, &imageMemory),"Failed to allocate image memory")

		VK_TEST(vkBindImageMemory(mDevice, image, imageMemory, 0), "Failed to bind image memory")
	}
	
	VkCommandBuffer Device::beginSingleTimeCommands() {
		VkCommandBufferAllocateInfo allocInfo = { };
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;
		allocInfo.commandPool = commandPool;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(mDevice, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);
		return commandBuffer;
	}
	
	void Device::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(graphicsQueue);

		vkFreeCommandBuffers(mDevice, commandPool, 1, &commandBuffer);
	}
}

