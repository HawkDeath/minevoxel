#pragma once


#include "Device.h"

#include <vulkan/vulkan.h>

#include <cassert>
#include <memory>
#include <string>
#include <vector>

namespace mv {
  class SwapChain
  {
  public:
    static constexpr auto MAX_FRAME_IN_FLIGHT = 2;
    SwapChain(Device& device, VkExtent2D extent);
    SwapChain(Device& device, VkExtent2D extent, std::shared_ptr<SwapChain> perviousSwapChain);
    ~SwapChain();

    SwapChain(const SwapChain&) = delete;
    SwapChain& operator=(const SwapChain&) = delete;

    VkFramebuffer getFrameBuffer(const int& idx) {
      assert(idx >= swapChainFramebuffers.size());
      return swapChainFramebuffers[idx];
    }

    VkRenderPass getRenderPass() {
      return renderPass;
    }

    VkImageView getImageView(const int& idx) {
      assert(idx >= swapChainImageViews.size());
      return swapChainImageViews[idx];
    }

    size_t imageCount() { return swapChainImages.size(); }

    VkFormat getSwapChainImageFormat() {
      return swapChainImageFormat;
    }

    VkExtent2D getSwapChainExtent() {
      return swapChainExtent;
    }

    std::uint32_t width() { return swapChainExtent.width; }
    std::uint32_t height() { return swapChainExtent.height; }

    float extentAspectRatio() {
      return static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height);
    }

    VkFormat findDepthFormat();

    VkResult acquireNextImage(std::uint32_t* imageIdx);
    VkResult submitCommandBuffers(const VkCommandBuffer* buffers, std::uint32_t* imageIdx);

    bool compareSwapFormats(const SwapChain& swapChain) const {
      return swapChain.swapChainImageFormat == swapChainImageFormat &&
        swapChain.swapChainDepthFormat == swapChainDepthFormat;
    }

  private:
    void initialize();

    void createSwapChain();
    void createImageViews();
    void createDepthResources();
    void createRenderPass();
    void createFramebuffers();
    void createSyncObjects();

  private:
    Device& mDevice;

    VkFormat swapChainImageFormat;
    VkFormat swapChainDepthFormat;
    VkExtent2D swapChainExtent;

    std::vector<VkFramebuffer> swapChainFramebuffers;
    VkRenderPass renderPass;

    std::vector<VkImage> depthImages;
    std::vector<VkDeviceMemory> depthImageMemorys;
    std::vector<VkImageView> depthImageViews;
    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;

    VkExtent2D windowExtent;

    VkSwapchainKHR swapChain;
    std::shared_ptr<SwapChain> oldSwapChain;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    std::vector<VkFence> imagesInFlight;
    size_t currentFrame = { 0 };
  };


}

