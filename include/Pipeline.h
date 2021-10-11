#pragma once

#include "Device.h"
#include <vulkan/vulkan.h>
#include <string>
#include <vector>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace mv {

	struct Vertex
	{
		glm::vec3 position = {};
		glm::vec3 color = {};
		glm::vec3 normal = {};
		glm::vec2 uv = {};

		static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
		static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

		bool operator==(const Vertex& other) const {
			return other.position == position && other.normal == normal && other.color == color && other.uv == uv;
		}

	};


	struct PipelineConfig
	{
		PipelineConfig() = default;
		PipelineConfig(const PipelineConfig&) = delete;
		PipelineConfig& operator=(const PipelineConfig&) = delete;

		VkPipelineViewportStateCreateInfo viewportInfo;
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
		VkPipelineRasterizationStateCreateInfo rasterizationInfo;
		VkPipelineMultisampleStateCreateInfo multisampleInfo;
		VkPipelineColorBlendAttachmentState colorBlendAttachment;
		VkPipelineColorBlendStateCreateInfo colorBlendInfo;
		VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
		std::vector<VkDynamicState> dynamicStateEnables;
		VkPipelineDynamicStateCreateInfo dynamicStateInfo;
		VkPipelineLayout pipelineLayout = { nullptr };
		VkRenderPass renderPass = { nullptr };
		std::uint32_t subpass = { 0 };
	};

	class Pipeline
	{
	public:
		Pipeline(Device &device, const std::string &vertexFilepath, const std::string &fragmentFilepath, const PipelineConfig &config);
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
