#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <cstdint>
#include <optional>
#include <vector>
#include "Log.h"

#define CUSTOM_ALLOCATOR nullptr
#define VK_TEST(func, msg) if (((func) != VK_SUCCESS)) {RT_THROW(msg);}

namespace mv {
  struct SwapChainSupportDetails
  {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
  };

  struct QueueFamilyIndices
  {
    std::optional<std::uint32_t> graphicsFamily;
    std::optional<std::uint32_t> presentFamily;

    bool isComplete() const noexcept {
      return graphicsFamily.has_value() && presentFamily.has_value();
    }
  };

  struct FrameInfo {
    VkCommandBuffer commandBuffer;
    VkDescriptorSet frameDescriptorSet;
  };

  namespace device_helper {
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR* surface);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR* surface);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device, const std::vector<const char*>& deviceExtensions);
    bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR* surface, const std::vector<const char*>& deviceExtensions);

  }

  namespace swapchain_helper {
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, VkExtent2D windowExtent = { 1280, 720 }); // baaad solution ;)
  }

  namespace pipeline_helper {
    std::vector<char> readBinaryFile(const std::string& filepath);
  }
}