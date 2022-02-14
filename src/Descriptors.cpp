#include "Descriptors.h"

#include "Log.h"

namespace mv {

DescriptorSetLayout::DescriptorSetLayout(Device &device) : mDevice{device} {}

DescriptorSetLayout::DescriptorSetLayout(
    Device &device,
    std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings)
    : mDevice{device}, mBindings{bindings} {
  createDescriptorSetLayout();
}

DescriptorSetLayout::~DescriptorSetLayout() {
  vkDestroyDescriptorSetLayout(mDevice.device(), mDescriptorSetLayout,
                               CUSTOM_ALLOCATOR);
}

void DescriptorSetLayout::addBinding(uint32_t binding,
                                     VkDescriptorType descriptor,
                                     VkShaderStageFlags stageFlags,
                                     uint32_t count) {
  assert(mBindings.count(binding) == 0 && "Binding alredy in use");

  VkDescriptorSetLayoutBinding layoutBinding = {};
  layoutBinding.binding = binding;
  layoutBinding.descriptorType = descriptor;
  layoutBinding.descriptorCount = count;
  layoutBinding.stageFlags = stageFlags;
  mBindings[binding] = layoutBinding;
}

void DescriptorSetLayout::createDescriptorSetLayout() {
  assert(mBindings.empty() && "Cannot create descriptor set without bindings");

  std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {};
  for (auto &binding : mBindings) {
    setLayoutBindings.push_back(binding.second);
  }

  VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo = {};
  descriptorSetLayoutInfo.sType =
      VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  descriptorSetLayoutInfo.bindingCount =
      static_cast<uint32_t>(setLayoutBindings.size());
  descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();

  VK_TEST(vkCreateDescriptorSetLayout(mDevice.device(),
                                      &descriptorSetLayoutInfo,
                                      CUSTOM_ALLOCATOR, &mDescriptorSetLayout),
          "Failed to create descriptor set layout")
}

//////////////////////////////////////////////////////////////////////////////////

DescriptorPool::DescriptorPool(Device &device) : mDevice{device} {}

DescriptorPool::DescriptorPool(
    Device &device, uint32_t maxSets, VkDescriptorPoolCreateFlags poolFlags,
    const std::vector<VkDescriptorPoolSize> &poolSizes)
    : mDevice{device}, mMaxSets{maxSets}, mPoolFlags{poolFlags},
      mPoolSizes{poolSizes} {
  createDescriptorPool();
}

DescriptorPool::~DescriptorPool() {
  vkDestroyDescriptorPool(mDevice.device(), mDescriptorPool, CUSTOM_ALLOCATOR);
}

void DescriptorPool::createDescriptorPool() {
  VkDescriptorPoolCreateInfo descriptorPoolInfo = {};
  descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(mPoolSizes.size());
  descriptorPoolInfo.pPoolSizes = mPoolSizes.data();
  descriptorPoolInfo.maxSets = mMaxSets;
  descriptorPoolInfo.flags = mPoolFlags;

  VK_TEST(vkCreateDescriptorPool(mDevice.device(), &descriptorPoolInfo,
                                 CUSTOM_ALLOCATOR, &mDescriptorPool),
          "Failed to craete descriptor pool");
}

bool DescriptorPool::allocateDescriptor(
    const VkDescriptorSetLayout descriptorSetLayout,
    VkDescriptorSet &descriptor) {
  VkDescriptorSetAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = mDescriptorPool;
  allocInfo.pSetLayouts = &descriptorSetLayout;
  allocInfo.descriptorSetCount = 1;

  if (vkAllocateDescriptorSets(mDevice.device(), &allocInfo, &descriptor) !=
      VK_SUCCESS) {
    ELOG("Failed to allocate descriptor sets");
    return false;
  }

  return true;
}

void DescriptorPool::freeDescriptors(
    std::vector<VkDescriptorSet> &descriptors) {
  vkFreeDescriptorSets(mDevice.device(), mDescriptorPool,
                       static_cast<uint32_t>(descriptors.size()),
                       descriptors.data());
}

void DescriptorPool::resetPool() {
  vkResetDescriptorPool(mDevice.device(), mDescriptorPool, 0);
}

//////////////////////////////////////////////////////////////////////////////////

DescriptorWriter::DescriptorWriter(DescriptorSetLayout &setLayout,
                                   DescriptorPool &pool)
    : mSetLayout{setLayout}, mPool{pool} {}

DescriptorWriter::~DescriptorWriter() {}

void DescriptorWriter::writeBuffer(uint32_t binding,
                                   VkDescriptorBufferInfo *bufferInfo) {
  assert(mSetLayout.mBindings.count(binding) == 1 &&
         "Layout does not contain specified binding");

  auto &bindingDesc = mSetLayout.mBindings[binding];

  assert(bindingDesc.descriptorCount == 1 &&
         "Binding single descriptor info, but binding expects multiple");

  VkWriteDescriptorSet write = {};
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.descriptorType = bindingDesc.descriptorType;
  write.dstBinding = binding;
  write.pBufferInfo = bufferInfo;
  write.descriptorCount = 1;

  mWriters.push_back(write);
}

void DescriptorWriter::writeImage(uint32_t binding,
                                  VkDescriptorImageInfo *imageInfo) {
  assert(mSetLayout.mBindings.count(binding) == 1 &&
         "Layout does not contain specified binding");

  auto &bindingDesc = mSetLayout.mBindings[binding];

  assert(bindingDesc.descriptorCount == 1 &&
         "Binding single descriptor info, but binding expects multiple");

  VkWriteDescriptorSet write = {};
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.descriptorType = bindingDesc.descriptorType;
  write.dstBinding = binding;
  write.pImageInfo = imageInfo;
  write.descriptorCount = 1;
  

  mWriters.push_back(write);
}

bool DescriptorWriter::build(VkDescriptorSet &set) {
  bool success =
      mPool.allocateDescriptor(mSetLayout.getDescriptorSetLayout(), set);
  if (!success) {
    ELOG("Failed to build descriptor writer");
    return false;
  }

  overwirte(set);
  return true;
}

void DescriptorWriter::overwirte(VkDescriptorSet &set) {
  for (auto &write : mWriters) {
    write.dstSet = set;
  }
  vkUpdateDescriptorSets(mPool.mDevice.device(),
                         static_cast<uint32_t>(mWriters.size()),
                         mWriters.data(), 0, nullptr);
}
} // namespace mv