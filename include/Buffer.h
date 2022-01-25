#pragma once

#include "Device.h"

#include <vulkan/vulkan.h>

namespace mv {
class Buffer
{
public:
    Buffer(Device &device);
    ~Buffer();

private:
    Device &mDevice;

};

Buffer::Buffer(Device &device) : mDevice {device}
{
}

Buffer::~Buffer()
{
}

}