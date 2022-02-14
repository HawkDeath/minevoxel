#pragma once

#include "Device.h"
#include "Pipeline.h"

#include <vulkan/vulkan.h>

#include <memory>

namespace mv {
class TestRenderSystem {
public:
  TestRenderSystem(Device &device, VkRenderPass renderPass,
                   VkDescriptorSetLayout globalSetLayout);
  ~TestRenderSystem();

  void render(VkCommandBuffer commandBuffer);

private:
  void createPipelineLayout(VkDescriptorSetLayout descriptorLayout);
  void createPipeline(VkRenderPass renderPass);

private:
  Device &mDevice;
  std::unique_ptr<Pipeline> mPipeline;
  VkPipelineLayout mPipelineLayout;
};

} // namespace mv