#pragma once

#include "Device.h"

#include <vulkan/vulkan.h>

#include <memory>
#include <unordered_map>
#include <vector>

namespace mv {
class DescriptorSetLayout {
  friend class DescriptorWriter;

public:
  DescriptorSetLayout(Device &device);
  DescriptorSetLayout(
      Device &device,
      std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);

  ~DescriptorSetLayout();

  DescriptorSetLayout(const DescriptorSetLayout &) = delete;
  DescriptorSetLayout &operator=(const DescriptorSetLayout &) = delete;

  void addBinding(uint32_t binding, VkDescriptorType descriptor,
                  VkShaderStageFlags stageFlags, uint32_t count = 1);
  void createDescriptorSetLayout();

  VkDescriptorSetLayout getDescriptorSetLayout() const {
    return mDescriptorSetLayout;
  }

private:
  Device &mDevice;
  VkDescriptorSetLayout mDescriptorSetLayout;
  std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> mBindings;
};

class DescriptorPool {
  friend class DescriptorWriter;

public:
  DescriptorPool(Device &device);
  DescriptorPool(Device &device, uint32_t maxSets,
                 VkDescriptorPoolCreateFlags poolFlags,
                 const std::vector<VkDescriptorPoolSize> &poolSizes);
  ~DescriptorPool();

  void addPoolSize(VkDescriptorType descriptorType, uint32_t count) {
    mPoolSizes.push_back({descriptorType, count});
  }
  void addPoolFlags(VkDescriptorPoolCreateFlags flags) { mPoolFlags = flags; }
  void setMaxSets(uint32_t maxSize) { mMaxSets = maxSize; }

  void createDescriptorPool();

  bool allocateDescriptor(const VkDescriptorSetLayout descriptorSetLayout,
                          VkDescriptorSet &descriptor);
  void freeDescriptors(std::vector<VkDescriptorSet> &descriptors);
  void resetPool();

private:
  Device &mDevice;
  VkDescriptorPool mDescriptorPool;
  VkDescriptorPoolCreateFlags mPoolFlags = {0};
  uint32_t mMaxSets = {1000}; // default is 1000 descriptors
  std::vector<VkDescriptorPoolSize> mPoolSizes;
};

class DescriptorWriter {
public:
  DescriptorWriter(DescriptorSetLayout &setLayout, DescriptorPool &pool);
  ~DescriptorWriter();

  void writeBuffer(uint32_t binding, VkDescriptorBufferInfo *bufferInfo);
  void writeImage(uint32_t binding, VkDescriptorImageInfo *imageInfo);

  bool build(VkDescriptorSet &set);
  void overwirte(VkDescriptorSet &set);

private:
  DescriptorSetLayout &mSetLayout;
  DescriptorPool &mPool;
  std::vector<VkWriteDescriptorSet> mWriters;
};

} // namespace mv