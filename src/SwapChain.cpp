#include "SwapChain.h"
#include "Log.h"

#include <array>

namespace mv {
  SwapChain::SwapChain(Device& device, VkExtent2D extent)
    : mDevice{ device }, windowExtent{ extent }
  {
    initialize();
  }

  SwapChain::SwapChain(Device& device, VkExtent2D extent, std::shared_ptr<SwapChain> perviousSwapChain)
    : mDevice{ device }, windowExtent{ extent }, oldSwapChain{ perviousSwapChain } {
    initialize();
    oldSwapChain = nullptr;
  }

  SwapChain::~SwapChain()
  {
    for (auto imageView : swapChainImageViews) {
      vkDestroyImageView(mDevice.device(), imageView, CUSTOM_ALLOCATOR);
    }
    swapChainImageViews.clear();

    if (swapChain != 0) {
      vkDestroySwapchainKHR(mDevice.device(), swapChain, CUSTOM_ALLOCATOR);
      swapChain = 0;
    }

    for (int i = 0; i < depthImages.size(); i++) {
      vkDestroyImageView(mDevice.device(), depthImageViews[i], CUSTOM_ALLOCATOR);
      vkDestroyImage(mDevice.device(), depthImages[i], CUSTOM_ALLOCATOR);
      vkFreeMemory(mDevice.device(), depthImageMemorys[i], CUSTOM_ALLOCATOR);
    }

    for (auto framebuffer : swapChainFramebuffers) {
      vkDestroyFramebuffer(mDevice.device(), framebuffer, CUSTOM_ALLOCATOR);
    }

    vkDestroyRenderPass(mDevice.device(), renderPass, CUSTOM_ALLOCATOR);

    for (size_t i = 0; i < MAX_FRAME_IN_FLIGHT; i++)
    {
      vkDestroySemaphore(mDevice.device(), renderFinishedSemaphores[i], CUSTOM_ALLOCATOR);
      vkDestroySemaphore(mDevice.device(), imageAvailableSemaphores[i], CUSTOM_ALLOCATOR);
      vkDestroyFence(mDevice.device(), inFlightFences[i], CUSTOM_ALLOCATOR);
    }
  }

  void SwapChain::initialize() {
    createSwapChain();
    createImageViews();
    createRenderPass();
    createDepthResources();
    createFramebuffers();
    createSyncObjects();
  }

  void SwapChain::createSwapChain() {
    auto swapChainSupport = mDevice.getSwapChainSupport();

    VkSurfaceFormatKHR surfaceFormt = swapchain_helper::chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = swapchain_helper::chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = swapchain_helper::chooseSwapExtent(swapChainSupport.capabilities, windowExtent);

    std::uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 &&
      imageCount > swapChainSupport.capabilities.maxImageCount) {
      imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = mDevice.getSurface();

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormt.format;
    createInfo.imageColorSpace = surfaceFormt.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    auto indices = mDevice.findPhysicalQueueFamily();
    std::uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };
    
    if (indices.graphicsFamily.value() != indices.presentFamily.value()) {
      createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
      createInfo.queueFamilyIndexCount = 2;
      createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else {
      createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
      createInfo.queueFamilyIndexCount = 0;
      createInfo.pQueueFamilyIndices = nullptr;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = oldSwapChain == nullptr ? VK_NULL_HANDLE : oldSwapChain->swapChain;

    if (VK_TEST(vkCreateSwapchainKHR(mDevice.device(), &createInfo, CUSTOM_ALLOCATOR, &swapChain))) {
      RT_THROW("Failed to create swap chain");
    }

    vkGetSwapchainImagesKHR(mDevice.device(), swapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(mDevice.device(), swapChain, &imageCount, swapChainImages.data());

    swapChainImageFormat = surfaceFormt.format;
    swapChainExtent = extent;
  }

  void SwapChain::createImageViews() {
    swapChainImageViews.resize(swapChainImages.size());
    for (size_t i = 0; i < swapChainImages.size(); i++)
    {
      VkImageViewCreateInfo viewInfo = {};
      viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
      viewInfo.image = swapChainImages[i];
      viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
      viewInfo.format = swapChainImageFormat;
      viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      viewInfo.subresourceRange.baseMipLevel = 0;
      viewInfo.subresourceRange.levelCount = 1;
      viewInfo.subresourceRange.baseArrayLayer = 0;
      viewInfo.subresourceRange.layerCount = 1;

      if (VK_TEST(vkCreateImageView(mDevice.device(), &viewInfo, CUSTOM_ALLOCATOR, &swapChainImageViews[i]))) {
        RT_THROW("Failed to create texture image view");
      }

    }
  }

  void SwapChain::createDepthResources() {
    VkFormat depthFormat = findDepthFormat();
    swapChainDepthFormat = depthFormat;
    VkExtent2D swapChainExtent = getSwapChainExtent();

    depthImages.resize(imageCount());
    depthImageMemorys.resize(imageCount());
    depthImageViews.resize(imageCount());

    for (int i = 0; i < depthImages.size(); i++)
    {
      VkImageCreateInfo imageInfo = {};
      imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
      imageInfo.imageType = VK_IMAGE_TYPE_2D;
      imageInfo.extent = { swapChainExtent.width, swapChainExtent.height, 1 };
      imageInfo.mipLevels = 1;
      imageInfo.arrayLayers = 1;
      imageInfo.format = depthFormat;
      imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
      imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
      imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
      imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
      imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
      imageInfo.flags = 0;

      mDevice.createImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImages[i], depthImageMemorys[i]);

      VkImageViewCreateInfo viewInfo = {};
      viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
      viewInfo.image = depthImages[i];
      viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
      viewInfo.format = depthFormat;
      viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
      viewInfo.subresourceRange.baseArrayLayer = 0;
      viewInfo.subresourceRange.baseMipLevel = 0;
      viewInfo.subresourceRange.levelCount = 1;
      viewInfo.subresourceRange.layerCount = 1;

      if (VK_TEST(vkCreateImageView(mDevice.device(), &viewInfo, CUSTOM_ALLOCATOR, &depthImageViews[i]))) {
        RT_THROW("Failed to create texture image view");
      }
    }
  }

  void SwapChain::createRenderPass() {
    VkAttachmentDescription depthAttachment = {};
    depthAttachment.format = findDepthFormat();
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef = {};
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depthAttachmentRef.attachment = 1;

    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = getSwapChainImageFormat();
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachmentRef.attachment = 0;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency subpassDeps = {};
    subpassDeps.dstSubpass = 0;
    subpassDeps.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    subpassDeps.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    subpassDeps.srcSubpass = VK_SUBPASS_EXTERNAL;
    subpassDeps.srcAccessMask = 0;
    subpassDeps.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

    std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<std::uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &subpassDeps;

    if (VK_TEST(vkCreateRenderPass(mDevice.device(), &renderPassInfo, CUSTOM_ALLOCATOR, &renderPass))) {
      RT_THROW("Failed to create render pass");
    }

  }

  void SwapChain::createFramebuffers() {
    swapChainFramebuffers.resize(imageCount());

    for (size_t i = 0; i < imageCount(); i++)
    {
      std::array<VkImageView, 2> attachments = { swapChainImageViews[i], depthImageViews[i] };

      VkExtent2D swapChainExtent = getSwapChainExtent();
      VkFramebufferCreateInfo framebufferInfo = {};
      framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
      framebufferInfo.renderPass = renderPass;
      framebufferInfo.attachmentCount = static_cast<std::uint32_t>(attachments.size());
      framebufferInfo.pAttachments = attachments.data();
      framebufferInfo.width = swapChainExtent.width;
      framebufferInfo.height = swapChainExtent.height;
      framebufferInfo.layers = 1;

      if (VK_TEST(vkCreateFramebuffer(mDevice.device(), &framebufferInfo, CUSTOM_ALLOCATOR, &swapChainFramebuffers[i]))) {
        RT_THROW("Failed to create framebuffers");
      }
    }
  }

  void SwapChain::createSyncObjects() {
    imageAvailableSemaphores.resize(MAX_FRAME_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAME_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAME_IN_FLIGHT);
    imagesInFlight.resize(imageCount(), VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAME_IN_FLIGHT; i++)
    {
      if (VK_TEST(vkCreateSemaphore(mDevice.device(), &semaphoreInfo, CUSTOM_ALLOCATOR, &imageAvailableSemaphores[i])) ||
        VK_TEST(vkCreateSemaphore(mDevice.device(), &semaphoreInfo, CUSTOM_ALLOCATOR, &renderFinishedSemaphores[i])) ||
        VK_TEST(vkCreateFence(mDevice.device(), &fenceInfo, CUSTOM_ALLOCATOR, &inFlightFences[i]))
        ) {
        RT_THROW("Failed to create synchronization objects for frame");
      }
    }
  }

  VkFormat SwapChain::findDepthFormat()
  {
    return mDevice.findSupportedFormat(
      { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
      VK_IMAGE_TILING_OPTIMAL,
      VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
  }
  VkResult SwapChain::acquireNextImage(std::uint32_t* imageIdx)
  {
    vkWaitForFences(mDevice.device(), 1, &inFlightFences[currentFrame], VK_TRUE, std::numeric_limits<std::uint64_t>::max());
    VkResult result = vkAcquireNextImageKHR(
      mDevice.device(), 
      swapChain, 
      std::numeric_limits<std::uint64_t>::max(),
      imageAvailableSemaphores[currentFrame],
      VK_NULL_HANDLE, imageIdx);
    return result;
  }

  VkResult SwapChain::submitCommandBuffers(const VkCommandBuffer* buffers, std::uint32_t* imageIdx)
  {
    if (imagesInFlight[*imageIdx] != VK_NULL_HANDLE) {
      vkWaitForFences(mDevice.device(), 1, &imagesInFlight[*imageIdx], VK_TRUE, std::numeric_limits<std::uint64_t>::max());
    }

    imagesInFlight[*imageIdx] = inFlightFences[currentFrame];

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = buffers;

    VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkResetFences(mDevice.device(), 1, &inFlightFences[currentFrame]);

    if (VK_TEST(vkQueueSubmit(mDevice.getGraphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]))) {
      RT_THROW("Failed to submit draw command buffer");
    }

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    
    VkSwapchainKHR swapChains[] = { swapChain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = imageIdx;

    auto result = vkQueuePresentKHR(mDevice.getGraphicsQueue(), &presentInfo);
    currentFrame = (currentFrame + 1) % MAX_FRAME_IN_FLIGHT;
    return result;
  }
}