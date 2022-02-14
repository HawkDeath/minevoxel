#pragma once

#include "Buffer.h"
#include "Device.h"

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

#include <memory>
#include <vector>

namespace mv {

struct Vertex {
  glm::vec3 position = {};
  glm::vec3 color = {};
  glm::vec3 normal = {};
  glm::vec2 uv = {};

  static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
  static std::vector<VkVertexInputAttributeDescription>
  getAttributeDescriptions();

  bool operator==(const Vertex &other) const {
    return position == other.position && normal == other.normal &&
           color == other.color && uv == other.uv;
  }
};

struct ModelLoader {
  std::vector<Vertex> vertices = {};
  std::vector<uint32_t> indices = {};

  void load(const std::string &filePath);
};

class Model {
public:
  Model(Device &device, const ModelLoader &loader);

  Model(const Model &) = delete;
  Model &operator=(const Model &) = delete;

  void bind(VkCommandBuffer commandBuffer);
  void draw(VkCommandBuffer commandBuffer);

private:
  void createVertexBuffer(const std::vector<Vertex> &vertices);
  void createIndexBuffer(const std::vector<uint32_t> &indices);

private:
  Device &mDevice;

  std::unique_ptr<Buffer> mVertexBuffer;
  uint32_t mVertexCount = {0};

  bool mHasIndices = {false};
  std::unique_ptr<Buffer> mIndexBuffer;
  uint32_t mIndexCount = {0};
};

} // namespace mv