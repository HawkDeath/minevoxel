#include "systems/ModelTestRenderSystem.h"

#include <vector>

namespace mv {    

    ModelTestRenderSystem::ModelTestRenderSystem(Device &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout) : mDevice{device}
    {
        createPipelineLayout(globalSetLayout);
        createPipline(renderPass);
    }
    
    ModelTestRenderSystem::~ModelTestRenderSystem()
    {
        vkDestroyPipelineLayout(mDevice.device(), mPipelineLayout, CUSTOM_ALLOCATOR);
    }

    void ModelTestRenderSystem::render(FrameInfo &frameInfo) {
        mPipeline->bind(frameInfo.commandBuffer);

        vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout, 0, 1, &frameInfo.frameDescriptorSet, 0, nullptr);
        
        // draw models
    }

    void ModelTestRenderSystem::createPipelineLayout(VkDescriptorSetLayout descriptorSetLayout) {
        std::vector<VkDescriptorSetLayout> descriptors{descriptorSetLayout};

        VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptors.size());
        pipelineLayoutInfo.pSetLayouts = descriptors.data();

        VK_TEST(vkCreatePipelineLayout(mDevice.device(), &pipelineLayoutInfo, CUSTOM_ALLOCATOR, &mPipelineLayout), "Failed to create pipeline layout")
    }
    
    void ModelTestRenderSystem::createPipline(VkRenderPass renderPass) {
        PipelineConfig pipelineConfig;
        Pipeline::defaultPipelineConfig(pipelineConfig);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = mPipelineLayout;

        mPipeline = std::make_unique<Pipeline>(mDevice, "shaders/model.vert.spv", "shaders/model.frag.spv", pipelineConfig);
    }
}