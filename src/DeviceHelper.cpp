#include "DeviceHelper.h"
#include "Log.h"
#include <algorithm>
#include <fstream>
#include <limits>
#include <set>
#include <string>

namespace mv {
  namespace device_helper {
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR* surface) {
      assert(surface != nullptr);
      SwapChainSupportDetails details;

      vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, *surface, &details.capabilities);

      std::uint32_t countFormats = 0;
      vkGetPhysicalDeviceSurfaceFormatsKHR(device, *surface, &countFormats, nullptr);

      if (countFormats != 0) {
        details.formats.resize(countFormats);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, *surface, &countFormats, details.formats.data());
      }

      std::uint32_t countPresentsMode = 0;
      vkGetPhysicalDeviceSurfacePresentModesKHR(device, *surface, &countPresentsMode, nullptr);

      if (countPresentsMode != 0) {
        details.presentModes.resize(countPresentsMode);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, *surface, &countPresentsMode, details.presentModes.data());
      }

      return details;
    }

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR* surface) {
      assert(surface != nullptr);
      QueueFamilyIndices indices;

      std::uint32_t countFamilyQueue = 0;
      vkGetPhysicalDeviceQueueFamilyProperties(device, &countFamilyQueue, nullptr);

      std::vector<VkQueueFamilyProperties> queueFamilies(countFamilyQueue);
      vkGetPhysicalDeviceQueueFamilyProperties(device, &countFamilyQueue, queueFamilies.data());

      size_t i = 0;
      for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
          indices.graphicsFamily = i;
        }
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, *surface, &presentSupport);
        if (queueFamily.queueCount > 0 && presentSupport) {
          indices.presentFamily = i;
        }

        if (indices.isComplete()) {
          break;
        }

        i++;
      }

      return indices;
    }

    bool checkDeviceExtensionSupport(VkPhysicalDevice device, const std::vector<const char*>& deviceExtensions) {
      std::uint32_t extensionsCount;
      vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionsCount, nullptr);

      std::vector<VkExtensionProperties> availableExtensions(extensionsCount);
      vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionsCount, availableExtensions.data());

      std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

      for (const auto& extension : availableExtensions)
      {
        requiredExtensions.erase(extension.extensionName);
      }
      bool result = requiredExtensions.empty();
      if (result) {
        for (const auto& ext : deviceExtensions) {
          LOG("Extensions has been found {}", ext);
        }
      }
      return result;
    }

    bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR* surface, const std::vector<const char*>& deviceExtensions) {
      auto indices = findQueueFamilies(device, surface);
      bool extensionsSupported = checkDeviceExtensionSupport(device, deviceExtensions);

      bool swapChainAdequate = { false };
      if (extensionsSupported) {
        auto swapChainSupport = querySwapChainSupport(device, surface);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
      }

      VkPhysicalDeviceFeatures supportedFeatures;
      vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

      return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
    }
  }

  namespace swapchain_helper {
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
      for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8_SRGB &&
          availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
          return availableFormat;
        }
      }
      return availableFormats[0];
    }

    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
      for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
          LOG("Present Mode: Mailbox");
          return availablePresentMode;
        }
      }

      LOG("Present Mode: FIFO (V-Sync)");
      return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, VkExtent2D windowExtent) {
      if (capabilities.currentExtent.width != std::numeric_limits<std::uint32_t>::max()) {
        return capabilities.currentExtent;
      }

      VkExtent2D actualExtent = windowExtent;
      actualExtent.width = std::max(capabilities.minImageExtent.width,
        std::min(capabilities.maxImageExtent.width, actualExtent.width));
      actualExtent.height = std::max(capabilities.minImageExtent.height,
        std::min(capabilities.maxImageExtent.height, actualExtent.height));

      return actualExtent;
    }
  }

  namespace pipeline_helper {
    std::vector<char> readBinaryFile(const std::string& filepath) {
      std::ifstream file{ filepath, std::ios_base::ate | std::ios_base::binary };

      if (!file.is_open()) {
        RT_THROW("Failed to open file: " + filepath);
      }

      auto fileSize = static_cast<size_t>(file.tellg());
      std::vector<char> buffer(fileSize);

      file.seekg(0);
      file.read(buffer.data(), fileSize);
      file.close();
      return buffer;
    }
  }
}