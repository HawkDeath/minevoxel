#include "Model.h"

#include "Log.h"
#include "DeviceHelper.h"

#include <cassert>
#include <cstring>

namespace mv {

	std::vector<VkVertexInputBindingDescription> Vertex::getBindingDescriptions()
	{
		std::vector<VkVertexInputBindingDescription> bindingDesc(1);
		bindingDesc[0].binding = 0;
		bindingDesc[0].stride = sizeof(Vertex);
		bindingDesc[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDesc;
	}

	std::vector<VkVertexInputAttributeDescription> Vertex::getAttributeDescriptions()
	{
		std::vector<VkVertexInputAttributeDescription> attribDesc{};
		attribDesc.push_back({ 0, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, position) });
		attribDesc.push_back({ 1, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, color) });
		attribDesc.push_back({ 2, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, normal) });
		attribDesc.push_back({ 3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv) });
		return attribDesc;
	}

	Model::Model(Device& device, const ModelLoader& loader) : mDevice{ device }
	{
		createVertexBuffer(loader.vertices);
		createIndexBuffer(loader.indices);
	}

	void Model::bind(VkCommandBuffer commandBuffer)
	{
		VkBuffer buffers[] = { mVertexBuffer->getBuffer() };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

		if (mHasIndices) {
			vkCmdBindIndexBuffer(commandBuffer, mIndexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
		}
	}

	void Model::draw(VkCommandBuffer commandBuffer)
	{
		if (mHasIndices) {
			vkCmdDrawIndexed(commandBuffer, mIndexCount, 1, 0, 0, 0);
		}
		else {
			vkCmdDraw(commandBuffer, mVertexCount, 1, 0, 0);
		}
	}

	void Model::createVertexBuffer(const std::vector<Vertex>& vertices)
	{
		mVertexCount = static_cast<uint32_t>(vertices.size());
		assert(mVertexCount >= 3 && "Vertex count must be at least 3");

		auto vertexSize = sizeof(vertices[0]);
		VkDeviceSize bufferSize = vertexSize * mVertexCount;
		
		Buffer staginBuffer = {
			mDevice,
			vertexSize,
			mVertexCount,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		};

		staginBuffer.map(bufferSize);
		staginBuffer.writeToBuffer((void*)vertices.data(), bufferSize);

		mVertexBuffer = std::make_unique<Buffer>(
			mDevice,
			vertexSize,
			mVertexCount,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
			);
		mDevice.copyBuffer(staginBuffer.getBuffer(), mVertexBuffer->getBuffer(), bufferSize);

	}

	void Model::createIndexBuffer(const std::vector<uint32_t>& indices)
	{
		mIndexCount = static_cast<uint32_t>(indices.size());
		mHasIndices = mIndexCount > 0;

		if (!mHasIndices) {
			return;
		}

		auto indexSize = sizeof(indices[0]);
		VkDeviceSize bufferSize = indexSize * mIndexCount;

		Buffer staginBuffer = {
			mDevice,
			indexSize,
			mIndexCount,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		};

		staginBuffer.map(bufferSize);
		staginBuffer.writeToBuffer((void*)indices.data(), bufferSize);

		mIndexBuffer = std::make_unique<Buffer>(
			mDevice,
			indexSize,
			mIndexCount,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
			);
		mDevice.copyBuffer(staginBuffer.getBuffer(), mIndexBuffer->getBuffer(), bufferSize);
	}
}