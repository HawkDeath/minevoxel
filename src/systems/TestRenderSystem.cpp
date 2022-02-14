#include "systems/TestRenderSystem.h"
#include "DeviceHelper.h"
#include "Log.h"

#include <vector>

namespace mv {

TestRenderSystem::TestRenderSystem(Device &device, VkRenderPass renderPass,
                                   VkDescriptorSetLayout globalSetLayout)
    : mDevice{device} {
  createPipelineLayout(globalSetLayout);
  createPipeline(renderPass);
}

TestRenderSystem::~TestRenderSystem() {
  vkDestroyPipelineLayout(mDevice.device(), mPipelineLayout, CUSTOM_ALLOCATOR);
}

void TestRenderSystem::render(VkCommandBuffer commandBuffer) {
  mPipeline->bind(commandBuffer);

  vkCmdDraw(commandBuffer, 3, 1, 0, 0);
}

void TestRenderSystem::createPipelineLayout(
    VkDescriptorSetLayout descriptorLayout) {
  std::vector<VkDescriptorSetLayout> descriptorLayouts{descriptorLayout};

  VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

  VK_TEST(vkCreatePipelineLayout(mDevice.device(), &pipelineLayoutInfo,
                                 CUSTOM_ALLOCATOR, &mPipelineLayout),
          "Failed to create pipeline layout")
}

void TestRenderSystem::createPipeline(VkRenderPass renderPass) {
  PipelineConfig config;
  Pipeline::defaultPipelineConfig(config);
  config.pipelineLayout = mPipelineLayout;
  config.renderPass = renderPass;
  mPipeline = std::make_unique<Pipeline>(mDevice, "shaders/triangle.vert.spv",
                                         "shaders/triangle.frag.spv", config);
}
} // namespace mv