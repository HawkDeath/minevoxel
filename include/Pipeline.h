#pragma once

#include "Device.h"
#include <vulkan/vulkan.h>
#include <string>
#include <vector>


namespace mv {

  struct PipelineConfig
  {
    PipelineConfig() = default;
    PipelineConfig(const PipelineConfig&) = delete;
    PipelineConfig& operator=(const PipelineConfig&) = delete;

    VkPipelineViewportStateCreateInfo viewportInfo = {};
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {};
    VkPipelineRasterizationStateCreateInfo rasterizationInfo = {};
    VkPipelineMultisampleStateCreateInfo multisampleInfo = {};
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    VkPipelineColorBlendStateCreateInfo colorBlendInfo = {};
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo = {};
    std::vector<VkDynamicState> dynamicStateEnables;
    VkPipelineDynamicStateCreateInfo dynamicStateInfo = {};
    VkPipelineLayout pipelineLayout = { 0 };
    VkRenderPass renderPass = { 0 };
    std::uint32_t subpass = { 0 };
  };

  class Pipeline
  {
  public:
    Pipeline(Device& device, const std::string& vertexFilepath, const std::string& fragmentFilepath, const PipelineConfig& config);
    ~Pipeline();

    void bind(VkCommandBuffer commandBuffer);

    static void defaultPipelineConfig(PipelineConfig& config) noexcept;

  private:
    void createGraphicsPipeline(const std::string& vertexFilepath, const std::string& fragmentFilepath, const PipelineConfig& config);
    void createShaderModule(const std::vector<char>& shaderBinary, VkShaderModule* shaderModule);

  private:
    Device& mDevice;

    VkPipeline graphicsPipeline;
    VkShaderModule vertexShaderModule;
    VkShaderModule fragmentShaderModule;
  };
}
