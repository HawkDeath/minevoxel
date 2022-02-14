#pragma once

#include "Device.h"

#include <vulkan/vulkan.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <string>

namespace mv {
	class Texture
	{
	public:
		Texture(Device &device, const std::string &textureFilePath);
		~Texture();



	private:
		void loadTextureFromFile(const std::string& filePath);
		void createTexture();
		void createImageBuffer();
		void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

	private:
		Device& mDevice;

		VkImage mImage;
		VkDeviceMemory mImageMemory;

		std::int32_t mWidth;
		std::int32_t mHeight;
		std::int32_t mChannels;
		stbi_uc* mImageRawData;
	};

}