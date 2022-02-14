#include "Model.h"

#include "DeviceHelper.h"
#include "Log.h"

#include <glm/gtx/hash.hpp>
#include <tiny_obj_loader.h>

#include <cassert>
#include <cstring>
#include <functional>

// from: https://stackoverflow.com/a/57595105
template <typename T, typename... Rest>
void hashCombine(std::size_t &seed, const T &v, const Rest &...rest) {
  seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  (hashCombine(seed, rest), ...);
};

namespace std {
template <> struct hash<mv::Vertex> {
  size_t operator()(mv::Vertex const &vertex) const {
    size_t seed = 0;
    hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
    return seed;
  }
};
} // namespace std

namespace mv {

std::vector<VkVertexInputBindingDescription> Vertex::getBindingDescriptions() {
  std::vector<VkVertexInputBindingDescription> bindingDesc(1);
  bindingDesc[0].binding = 0;
  bindingDesc[0].stride = sizeof(Vertex);
  bindingDesc[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
  return bindingDesc;
}

std::vector<VkVertexInputAttributeDescription>
Vertex::getAttributeDescriptions() {
  std::vector<VkVertexInputAttributeDescription> attribDesc{};
  attribDesc.push_back(
      {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)});
  attribDesc.push_back(
      {1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)});
  attribDesc.push_back(
      {2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)});
  attribDesc.push_back({3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)});
  return attribDesc;
}

Model::Model(Device &device, const ModelLoader &loader) : mDevice{device} {
  createVertexBuffer(loader.vertices);
  createIndexBuffer(loader.indices);
}

void Model::bind(VkCommandBuffer commandBuffer) {
  VkBuffer buffers[] = {mVertexBuffer->getBuffer()};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

  if (mHasIndices) {
    vkCmdBindIndexBuffer(commandBuffer, mIndexBuffer->getBuffer(), 0,
                         VK_INDEX_TYPE_UINT32);
  }
}

void Model::draw(VkCommandBuffer commandBuffer) {
  if (mHasIndices) {
    vkCmdDrawIndexed(commandBuffer, mIndexCount, 1, 0, 0, 0);
  } else {
    vkCmdDraw(commandBuffer, mVertexCount, 1, 0, 0);
  }
}

void Model::createVertexBuffer(const std::vector<Vertex> &vertices) {
  mVertexCount = static_cast<uint32_t>(vertices.size());
  assert(mVertexCount >= 3 && "Vertex count must be at least 3");

  auto vertexSize = sizeof(vertices[0]);
  VkDeviceSize bufferSize = vertexSize * mVertexCount;

  Buffer staginBuffer = {mDevice, vertexSize, mVertexCount,
                         VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                             VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};

  staginBuffer.map(bufferSize);
  staginBuffer.writeToBuffer((void *)vertices.data(), bufferSize);

  mVertexBuffer = std::make_unique<Buffer>(mDevice, vertexSize, mVertexCount,
                                           VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                                               VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  mDevice.copyBuffer(staginBuffer.getBuffer(), mVertexBuffer->getBuffer(),
                     bufferSize);
}

void Model::createIndexBuffer(const std::vector<uint32_t> &indices) {
  mIndexCount = static_cast<uint32_t>(indices.size());
  mHasIndices = mIndexCount > 0;

  if (!mHasIndices) {
    return;
  }

  auto indexSize = sizeof(indices[0]);
  VkDeviceSize bufferSize = indexSize * mIndexCount;

  Buffer staginBuffer = {mDevice, indexSize, mIndexCount,
                         VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                             VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};

  staginBuffer.map(bufferSize);
  staginBuffer.writeToBuffer((void *)indices.data(), bufferSize);

  mIndexBuffer = std::make_unique<Buffer>(mDevice, indexSize, mIndexCount,
                                          VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
                                              VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  mDevice.copyBuffer(staginBuffer.getBuffer(), mIndexBuffer->getBuffer(),
                     bufferSize);
}

void ModelLoader::load(const std::string &filePath) {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;

  if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                        filePath.c_str())) {
    ELOG("Failed to load file {}: {}{}", filePath, warn, err);
    return;
  }

  vertices.clear();
  indices.clear();

  std::unordered_map<Vertex, uint32_t> uniqueVertices = {};

  for (const auto &shape : shapes) {
    for (const auto index : shape.mesh.indices) {
      Vertex vertex = {};

      if (index.vertex_index >= 0) {
        vertex.position = {attrib.vertices[3 * index.vertex_index + 0],
                           attrib.vertices[3 * index.vertex_index + 1],
                           attrib.vertices[3 * index.vertex_index + 2]};
        vertex.color = {attrib.colors[3 * index.vertex_index + 0],
                        attrib.colors[3 * index.vertex_index + 1],
                        attrib.colors[3 * index.vertex_index + 2]};
      } // vertex_index

      if (index.normal_index >= 0) {
        vertex.normal = {attrib.normals[3 * index.normal_index + 0],
                         attrib.normals[3 * index.normal_index + 1],
                         attrib.normals[3 * index.normal_index + 2]};
      } // normal_index

      if (index.texcoord_index >= 0) {
        vertex.uv = {attrib.texcoords[2 * index.normal_index + 0],
                     attrib.texcoords[2 * index.normal_index + 1]};
      } // texcoord_index

      if (uniqueVertices.count(vertex) == 0) {
        uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
        vertices.push_back(vertex);
      }
      indices.push_back(uniqueVertices[vertex]);
    } // index
  }   // shape
}
} // namespace mv