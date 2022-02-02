#include "Pipeline.h"
#include "Log.h"
#include "DeviceHelper.h"

#include "Model.h"

#include <cassert>

namespace mv {


  Pipeline::Pipeline(Device& device, const std::string& vertexFilepath, const std::string& fragmentFilepath, const PipelineConfig& config)
    : mDevice{ device }
  {
    createGraphicsPipeline(vertexFilepath, fragmentFilepath, config);
  }

  Pipeline::~Pipeline()
  {
    vkDestroyShaderModule(mDevice.device(), vertexShaderModule, CUSTOM_ALLOCATOR);
    vkDestroyShaderModule(mDevice.device(), fragmentShaderModule, CUSTOM_ALLOCATOR);
    vkDestroyPipeline(mDevice.device(), graphicsPipeline, CUSTOM_ALLOCATOR);
  }

  void Pipeline::bind(VkCommandBuffer commandBuffer)
  {
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
  }

  void Pipeline::defaultPipelineConfig(PipelineConfig& config) noexcept
  {
    config.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    config.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    config.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

    config.viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    config.viewportInfo.viewportCount = 1;
    config.viewportInfo.pViewports = nullptr;
    config.viewportInfo.scissorCount = 1;
    config.viewportInfo.pScissors = nullptr;

    config.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    config.rasterizationInfo.depthClampEnable = VK_FALSE;
    config.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
    config.rasterizationInfo.lineWidth = 1.0f;
    config.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
    config.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    config.rasterizationInfo.depthBiasEnable = VK_FALSE;
    config.rasterizationInfo.depthBiasConstantFactor = 0.0f;
    config.rasterizationInfo.depthBiasClamp = 0.0f;
    config.rasterizationInfo.depthBiasSlopeFactor = 0.0f;

    config.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    config.multisampleInfo.sampleShadingEnable = VK_FALSE;
    config.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    config.multisampleInfo.minSampleShading = 1.0f;
    config.multisampleInfo.pSampleMask = nullptr;
    config.multisampleInfo.alphaToCoverageEnable = VK_FALSE;
    config.multisampleInfo.alphaToOneEnable = VK_FALSE;

    config.colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    config.colorBlendAttachment.blendEnable = VK_FALSE;
    config.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    config.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    config.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    config.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    config.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    config.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    config.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    config.colorBlendInfo.logicOpEnable = VK_FALSE;
    config.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;
    config.colorBlendInfo.attachmentCount = 1;
    config.colorBlendInfo.pAttachments = &config.colorBlendAttachment;
    config.colorBlendInfo.blendConstants[0] = 0.0f;
    config.colorBlendInfo.blendConstants[1] = 0.0f;
    config.colorBlendInfo.blendConstants[2] = 0.0f;
    config.colorBlendInfo.blendConstants[3] = 0.0f;

    config.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    config.depthStencilInfo.depthTestEnable = VK_TRUE;
    config.depthStencilInfo.depthWriteEnable = VK_TRUE;
    config.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
    config.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
    config.depthStencilInfo.minDepthBounds = 0.0f;
    config.depthStencilInfo.maxDepthBounds = 1.0f;
    config.depthStencilInfo.stencilTestEnable = VK_FALSE;
    config.depthStencilInfo.front = {};
    config.depthStencilInfo.back = {};

    config.dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    config.dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    config.dynamicStateInfo.pDynamicStates = config.dynamicStateEnables.data();
    config.dynamicStateInfo.dynamicStateCount = static_cast<std::uint32_t>(config.dynamicStateEnables.size());
    config.dynamicStateInfo.flags = 0;
  }

  void Pipeline::createGraphicsPipeline(const std::string& vertexFilepath, const std::string& fragmentFilepath, const PipelineConfig& config)
  {
    assert(config.renderPass != VK_NULL_HANDLE &&
      "Cannot create graphics pipeline: no renderPass provided in configPipeline");
    assert(config.pipelineLayout != VK_NULL_HANDLE &&
      "Cannot create graphics pipeline: no pipelineLayout provided in configPipeline");

    auto vertCode = pipeline_helper::readBinaryFile(vertexFilepath);
    auto fragCode = pipeline_helper::readBinaryFile(fragmentFilepath);

    createShaderModule(vertCode, &vertexShaderModule);
    createShaderModule(fragCode, &fragmentShaderModule);

    VkPipelineShaderStageCreateInfo shaderStages[2];
    shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStages[0].module = vertexShaderModule;
    shaderStages[0].pName = "main";
    shaderStages[0].flags = 0;
    shaderStages[0].pNext = nullptr;
    shaderStages[0].pSpecializationInfo = nullptr;
    shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStages[1].module = fragmentShaderModule;
    shaderStages[1].pName = "main";
    shaderStages[1].flags = 0;
    shaderStages[1].pNext = nullptr;
    shaderStages[1].pSpecializationInfo = nullptr;

    auto bindingDesc = Vertex::getBindingDescriptions();
    auto attribDesc = Vertex::getAttributeDescriptions();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<std::uint32_t>(attribDesc.size());
    vertexInputInfo.vertexBindingDescriptionCount = static_cast<std::uint32_t>(bindingDesc.size());
    vertexInputInfo.pVertexAttributeDescriptions = attribDesc.data();
    vertexInputInfo.pVertexBindingDescriptions = bindingDesc.data();

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &config.inputAssemblyInfo;
    pipelineInfo.pViewportState = &config.viewportInfo;
    pipelineInfo.pRasterizationState = &config.rasterizationInfo;
    pipelineInfo.pMultisampleState = &config.multisampleInfo;
    pipelineInfo.pColorBlendState = &config.colorBlendInfo;
    pipelineInfo.pDepthStencilState = &config.depthStencilInfo;
    pipelineInfo.pDynamicState = &config.dynamicStateInfo;

    pipelineInfo.layout = config.pipelineLayout;
    pipelineInfo.renderPass = config.renderPass;
    pipelineInfo.subpass = config.subpass;

    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    VK_TEST(vkCreateGraphicsPipelines(mDevice.device(), VK_NULL_HANDLE, 1, &pipelineInfo, CUSTOM_ALLOCATOR, &graphicsPipeline),
      "Failed to create graphics pipeline")
  }

  void Pipeline::createShaderModule(const std::vector<char>& shaderBinary, VkShaderModule* shaderModule)
  {
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = shaderBinary.size();
    createInfo.pCode = reinterpret_cast<const std::uint32_t*>(shaderBinary.data());

    VK_TEST(vkCreateShaderModule(mDevice.device(), &createInfo, CUSTOM_ALLOCATOR, shaderModule), "Failed to create shader module")
  }
}