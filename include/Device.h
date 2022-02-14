#pragma once

#include "DeviceHelper.h"
#include "Window.h"
#include <vector>
#include <vulkan/vulkan.h>

namespace mv {
class Device {
public:
  explicit Device(Window &window);
  ~Device();

  Device(const Device &) = delete;
  Device &operator=(const Device &) = delete;
  Device(Device &&) = delete;
  Device &operator=(Device &&) = delete;

  VkCommandPool getCommandPool() const { return commandPool; }

  VkSurfaceKHR getSurface() const { return surface; }

  VkDevice device() const { return mDevice; }

  VkQueue getGraphicsQueue() const { return graphicsQueue; }

  VkQueue getPresentQueue() const { return presentQueue; }

  SwapChainSupportDetails getSwapChainSupport() {
    return device_helper::querySwapChainSupport(physicalDevice, &surface);
  }

  std::uint32_t findMemoryType(std::uint32_t typeFilter,
                               VkMemoryPropertyFlags properties);

  QueueFamilyIndices findPhysicalQueueFamily() {
    return device_helper::findQueueFamilies(physicalDevice, &surface);
  }

  VkFormat findSupportedFormat(const std::vector<VkFormat> &formats,
                               VkImageTiling tiling,
                               VkFormatFeatureFlags features);

  void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                    VkMemoryPropertyFlags properties, VkBuffer &buffer,
                    VkDeviceMemory &bufferMemory);
  void copyBuffer(VkBuffer scrBuffer, VkBuffer dstBuffer, VkDeviceSize size);
  void copyBufferToImage(VkBuffer buffer, VkImage image, std::uint32_t width,
                         std::uint32_t height, std::uint32_t layerCount);
  void createImageWithInfo(const VkImageCreateInfo &imageInfo,
                           VkMemoryPropertyFlags properties, VkImage &image,
                           VkDeviceMemory &imageMemory);

  VkCommandBuffer beginSingleTimeCommands();
  void endSingleTimeCommands(VkCommandBuffer commandBuffer);

  VkPhysicalDeviceProperties properties;

private:
  void createInstance();
  void setupDebugMsg();
  void createSurface();
  void pickPhysicalDevice();
  void createLogicalDevice();
  void createCommandPool();

  std::vector<const char *> getRequiredExtensions();
  void checkRequiredInstanceExtentions();
  void populateDebugMessengerCreateInfo(
      VkDebugUtilsMessengerCreateInfoEXT &createInfo);
  bool checkValidationLayerSupport();

private:
  Window &mWindow;

  VkInstance instance;
  VkDebugUtilsMessengerEXT debugMessenger;
  VkPhysicalDevice physicalDevice = {VK_NULL_HANDLE};
  VkSurfaceKHR surface;
  VkDevice mDevice;

  VkCommandPool commandPool;
  VkQueue graphicsQueue;
  VkQueue presentQueue;

  bool enableValidationLayers = {true};

  const std::vector<const char *> validationLayers = {
      "VK_LAYER_KHRONOS_validation"};
  const std::vector<const char *> deviceExtensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME};
};
} // namespace mv