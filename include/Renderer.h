#pragma once

#include "Device.h"
#include "Window.h"
#include "SwapChain.h"

#include <vulkan/vulkan.h>

#include <memory>
#include <vector>
#include <cassert>

namespace mv {
	class Renderer
	{
	public:
		Renderer(Window &window, Device &device);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;

		VkRenderPass getSwapChainRenderPass() const { return swapChain->getRenderPass(); };

		float getAspectRatio() const { return swapChain->extentAspectRatio(); }
		bool isFrameInProgress() const { return isFrameStarted; }

		VkCommandBuffer getCurrentCommandBuffer() const {
			assert(isFrameStarted && "Cannot get command buffer when frame is not in progress");
			return commandBuffers[currentFrameIdx];
		}

		int getFrameIndex() const {
			assert(isFrameStarted && "Cannot get frame index when frame is not in progress");
			return currentFrameIdx;
		}

		VkCommandBuffer beginFrame();
		void endFrame();
		void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
		void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

	private:
		void createCommanBuffers();
		void freeCommandBuffers();
		void recreateSwapChain();

	private:
		Window& mWindow;
		Device& mDevice;

		std::unique_ptr<SwapChain> swapChain;
		std::vector<VkCommandBuffer> commandBuffers;

		std::uint32_t currentImageIdx;
		int currentFrameIdx = { 0 };
		bool isFrameStarted = { false };
	};
}
