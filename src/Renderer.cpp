#include "Renderer.h"
#include "Log.h"
#include <array>

namespace mv {
  Renderer::Renderer(Window& window, Device& device) : mWindow{ window }, mDevice{ device }
  {
    recreateSwapChain();
    createCommanBuffers();
  }

  Renderer::~Renderer()
  {
    freeCommandBuffers();
  }

  VkCommandBuffer Renderer::beginFrame()
  {
    assert(!isFrameStarted && "Can't call beginFrame while already in progress");

    auto result = swapChain->acquireNextImage(&currentImageIdx);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
      recreateSwapChain();
      return nullptr;
    }

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
      RT_THROW("Failed to acquire swap chain image");
    }

    isFrameStarted = true;

    auto commandBuffer = getCurrentCommandBuffer();
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (VK_TEST(vkBeginCommandBuffer(commandBuffer, &beginInfo))) {
      RT_THROW("Failed to begin recording command buffer");
    }

    return commandBuffer;
  }

  void Renderer::endFrame()
  {
    assert(!isFrameStarted && "Can't call endFrame while already in progress");

    auto commandBuffer = getCurrentCommandBuffer();

    if (VK_TEST(vkEndCommandBuffer(commandBuffer))) {
      RT_THROW("Failed to record command buffer");
    }

    auto result = swapChain->submitCommandBuffers(&commandBuffer, &currentImageIdx);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR // TODO: add '|| window has been resized'
      ) {
      recreateSwapChain();
    }
    else if (result != VK_SUCCESS) {
      RT_THROW("Failed to present swap chain image");
    }

    isFrameStarted = false;
    currentFrameIdx = (currentFrameIdx + 1) % SwapChain::MAX_FRAME_IN_FLIGHT;
  }

  void Renderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer)
  {
    assert(isFrameStarted && "Can't call beginSwapChainRenderPass while frame is not in progress");
    assert(commandBuffer == getCurrentCommandBuffer() && "Can't begin render pass on command buffer from a different frame");

    VkRenderPassBeginInfo renderPassInfo = { };
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = swapChain->getRenderPass();
    renderPassInfo.framebuffer = swapChain->getFrameBuffer(currentImageIdx);

    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = swapChain->getSwapChainExtent();

    std::array<VkClearValue, 2> clearValues = {};
    clearValues[0].color = { 0.125f, 0.125f, 0.125f, 1.0f };
    clearValues[1].depthStencil = { 1.0f, 0 };
    renderPassInfo.clearValueCount = static_cast<std::uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapChain->getSwapChainExtent().width);
    viewport.height = static_cast<float>(swapChain->getSwapChainExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = { {0, 0}, swapChain->getSwapChainExtent() };

    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
  }

  void Renderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer)
  {
    assert(isFrameStarted && "Can't call beginSwapChainRenderPass while frame is not in progress");
    assert(commandBuffer == getCurrentCommandBuffer() && "Can't end render pass on command buffer from a different frame");
    vkCmdEndRenderPass(commandBuffer);
  }

  void Renderer::createCommanBuffers()
  {
    commandBuffers.resize(SwapChain::MAX_FRAME_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = mDevice.getCommandPool();
    allocInfo.commandBufferCount = static_cast<std::uint32_t>(commandBuffers.size());

    if (VK_TEST(vkAllocateCommandBuffers(mDevice.device(), &allocInfo, commandBuffers.data()))) {
      RT_THROW("Failed to allocate command buffers");
    }
  }

  void Renderer::freeCommandBuffers()
  {
    vkFreeCommandBuffers(mDevice.device(), mDevice.getCommandPool(), static_cast<std::uint32_t>(commandBuffers.size()), commandBuffers.data());
    commandBuffers.clear();
  }

  void Renderer::recreateSwapChain()
  {
    auto extent = mWindow.getExtent2D();
    while (extent.width == 0 || extent.height == 0)
    {
      extent = mWindow.getExtent2D();
      glfwWaitEvents();
    }
    vkDeviceWaitIdle(mDevice.device());

    if (swapChain == nullptr) {
      swapChain = std::make_unique<SwapChain>(mDevice, extent);
    }
    else {
      std::shared_ptr<SwapChain> oldSwapChain = std::move(swapChain);
      swapChain = std::make_unique<SwapChain>(mDevice, extent, oldSwapChain);

      if (!oldSwapChain->compareSwapFormats(*swapChain.get())) {
        RT_THROW("Swap chain image(or depth) format has changed");
      }
    }
  }
}