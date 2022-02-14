#pragma once

#include "Device.h"

#include <vulkan/vulkan.h>

#include <stb_image.h>

#include <string>

namespace mv {
  class Texture {
  public:
    Texture(Device& device, const std::string& textureFilePath, VkFormat format);
    ~Texture();


    VkImageView getImageView() const {
      return mImageView;
    }

    VkSampler getSampler() const {
      return mImageSampler;
    }

  private:
    void loadTextureFromFile(const std::string& filePath);
    void createTexture();
    void createImageView();
    void createTextureSampler();
    void createImageBuffer();
    void transitionImageLayout(VkImage image, VkFormat format,
      VkImageLayout oldLayout, VkImageLayout newLayout);

  private:
    Device& mDevice;

    VkImage mImage;
    VkDeviceMemory mImageMemory;
    VkImageView mImageView;
    VkSampler mImageSampler;
    VkFormat mFormat;

    std::int32_t mWidth;
    std::int32_t mHeight;
    std::int32_t mChannels;
    stbi_uc* mImageRawData;
  };

} // namespace mv