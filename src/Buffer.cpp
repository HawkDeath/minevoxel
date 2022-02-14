#include "Buffer.h"

#include <cassert>
#include <cstring>

namespace mv {

Buffer::Buffer(Device &device, VkDeviceSize instanceSize,
               uint32_t instanceCount, VkBufferUsageFlags usageFlags,
               VkMemoryPropertyFlags memPropFlags,
               VkDeviceSize minOffsetAlignment)
    : mDevice{device}, mInstanceSize{instanceSize},
      mInstanceCount{instanceCount}, mUsageFlags{usageFlags},
      mMemoryPropertyFlags{memPropFlags} {

  mAlignmentSize = getAlignment(instanceSize, minOffsetAlignment);
  mBufferSize = mAlignmentSize * mInstanceCount;
  mDevice.createBuffer(mBufferSize, usageFlags, memPropFlags, mBuffer, mMemory);
}

Buffer::~Buffer() {
  unMap();
  vkDestroyBuffer(mDevice.device(), mBuffer, CUSTOM_ALLOCATOR);
  vkFreeMemory(mDevice.device(), mMemory, CUSTOM_ALLOCATOR);
}

VkResult Buffer::map(VkDeviceSize size, VkDeviceSize offset) {
  assert(mBuffer && mMemory && "Called map on buffer before create");
  return vkMapMemory(mDevice.device(), mMemory, offset, size, 0, &mMapped);
}

void Buffer::unMap() {
  if (mMapped) {
    vkUnmapMemory(mDevice.device(), mMemory);
    mMapped = nullptr;
  }
}

void Buffer::writeToBuffer(void *data, VkDeviceSize size, VkDeviceSize offset) {
  assert(mMapped && "Cannot write to unmapped buffer");

  char *memOffset = (char *)mMapped;
  memOffset += offset;
  memcpy(memOffset, data, size);
}

VkResult Buffer::flush(VkDeviceSize size, VkDeviceSize offset) {
  VkMappedMemoryRange mappedRange = {};
  mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
  mappedRange.memory = mMemory;
  mappedRange.offset = offset;
  mappedRange.size = size;
  return vkFlushMappedMemoryRanges(mDevice.device(), 1, &mappedRange);
}

VkDescriptorBufferInfo Buffer::descriptorInfo(VkDeviceSize size,
                                              VkDeviceSize offset) {
  VkDescriptorBufferInfo descBuffInfo = {};
  descBuffInfo.buffer = mBuffer;
  descBuffInfo.offset = offset;
  descBuffInfo.range = size;
  return descBuffInfo;
}

VkResult Buffer::invalidate(VkDeviceSize size, VkDeviceSize offset) {
  VkMappedMemoryRange mappedRange = {};
  mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
  mappedRange.memory = mMemory;
  mappedRange.offset = offset;
  mappedRange.size = size;
  return vkInvalidateMappedMemoryRanges(mDevice.device(), 1, &mappedRange);
}

void Buffer::writeToIndex(void *data, int32_t idx) {
  writeToBuffer(data, mInstanceSize, idx * mAlignmentSize);
}

VkResult Buffer::flushToIndex(int32_t idx) {
  return flush(mAlignmentSize, idx * mAlignmentSize);
}

VkDescriptorBufferInfo Buffer::descriptorInfoIndex(int32_t idx) {
  return descriptorInfo(mAlignmentSize, idx * mAlignmentSize);
}

VkResult Buffer::invalidateIndex(int32_t idx) {
  return invalidate(mAlignmentSize, idx * mAlignmentSize);
}

VkDeviceSize Buffer::getAlignment(VkDeviceSize instanceSize,
                                  VkDeviceSize minOffsetAlignment) {
  if (minOffsetAlignment > 0) {
    return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
  }
  return instanceSize;
}

} // namespace mv