#pragma once

#include "Device.h"

#include <vulkan/vulkan.h>

namespace mv {
class Buffer {
public:
  Buffer(Device &device, VkDeviceSize instanceSize, uint32_t instanceCount,
         VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memPropFlags,
         VkDeviceSize minOffsetAlignment = 1);
  ~Buffer();

  Buffer(const Buffer &) = delete;
  Buffer &operator=(const Buffer &) = delete;

  VkResult map(VkDeviceSize size, VkDeviceSize offset = 0);
  void unMap();

  void writeToBuffer(void *data, VkDeviceSize size, VkDeviceSize offset = 0);
  VkResult flush(VkDeviceSize size, VkDeviceSize offset = 0);
  VkDescriptorBufferInfo descriptorInfo(VkDeviceSize size,
                                        VkDeviceSize offset = 0);
  VkResult invalidate(VkDeviceSize size, VkDeviceSize offset = 0);

  void writeToIndex(void *data, int32_t idx);
  VkResult flushToIndex(int32_t idx);
  VkDescriptorBufferInfo descriptorInfoIndex(int32_t idx);
  VkResult invalidateIndex(int32_t idx);

  void *getMappedMemory() const { return mMapped; }
  VkBuffer getBuffer() const { return mBuffer; }
  VkDeviceMemory getMemory() const { return mMemory; }

  VkDeviceSize getBufferSize() const { return mBufferSize; }
  uint32_t getInstanceCount() const { return mInstanceCount; }
  VkDeviceSize getInstanceSize() const { return mInstanceSize; }
  VkDeviceSize getAlignmentSize() const { return mAlignmentSize; }
  VkBufferUsageFlags getUsageFlags() const { return mUsageFlags; }
  VkMemoryPropertyFlags getMemoryPropertyFlags() const {
    return mMemoryPropertyFlags;
  }

private:
  static VkDeviceSize getAlignment(VkDeviceSize instanceSize,
                                   VkDeviceSize minOffsetAlignment);

private:
  Device &mDevice;

  void *mMapped = nullptr;
  VkBuffer mBuffer;
  VkDeviceMemory mMemory;

  VkDeviceSize mBufferSize;
  uint32_t mInstanceCount;
  VkDeviceSize mInstanceSize;
  VkDeviceSize mAlignmentSize;
  VkBufferUsageFlags mUsageFlags;
  VkMemoryPropertyFlags mMemoryPropertyFlags;
};

} // namespace mv