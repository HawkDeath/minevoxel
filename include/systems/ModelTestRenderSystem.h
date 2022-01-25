#pragma once

#include "Device.h"
#include "Pipeline.h"

#include <vulkan/vulkan.h>

#include <memory>


namespace mv {
    class ModelTestRenderSystem
    {
    public:
        ModelTestRenderSystem(Device &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
        ~ModelTestRenderSystem();

        void render(FrameInfo &frameInfo);

    private:
        void createPipelineLayout(VkDescriptorSetLayout descriptorSetLayout);
        void createPipline(VkRenderPass renderPass);

    private:
        Device &mDevice;
        std::unique_ptr<Pipeline> mPipeline;
        VkPipelineLayout mPipelineLayout;
    };
}