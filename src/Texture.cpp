#include "Texture.h"
#include "Buffer.h"

#include "Log.h"

#include <cassert>

namespace mv {

	Texture::Texture(Device& device, const std::string& textureFilePath) : mDevice{ device }
	{
		loadTextureFromFile(textureFilePath);
		createTexture();
	}

	Texture::~Texture()
	{
		vkDestroyImage(mDevice.device(), mImage, CUSTOM_ALLOCATOR);
		vkFreeMemory(mDevice.device(), mImageMemory, CUSTOM_ALLOCATOR);
	}


	void Texture::loadTextureFromFile(const std::string& filePath)
	{
		mImageRawData = stbi_load(filePath.c_str(), &mWidth, &mHeight, &mChannels, STBI_rgb);
		if (!mImageRawData) {
			RT_THROW("Failed to load texture image");
		}
	}


	void Texture::createTexture()
	{
		assert(mImageRawData && "Must load image before allocate memory on GPU");

		VkDeviceSize imageSize = mWidth * mHeight * 3; // 3 - case rgb, not rgba

		Buffer stagingBuffer = {
			mDevice, imageSize, 1, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		};

		stagingBuffer.map(imageSize);
		stagingBuffer.writeToBuffer((void*)mImageRawData, imageSize);

		stbi_image_free(mImageRawData); // not needed in RAM

		createImageBuffer();

		transitionImageLayout(mImage, VK_FORMAT_R8G8B8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		mDevice.copyBufferToImage(stagingBuffer.getBuffer(), mImage, mWidth, mHeight, 1);
		transitionImageLayout(mImage, VK_FORMAT_R8G8B8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	}

	void Texture::createImageBuffer() {
		VkImageCreateInfo imageInfo = {};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = mWidth;
		imageInfo.extent.height = mHeight;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = VK_FORMAT_R8G8B8_SRGB;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VK_TEST(vkCreateImage(mDevice.device(), &imageInfo, CUSTOM_ALLOCATOR, &mImage), "Failed to create image");

		VkMemoryRequirements memReq;
		vkGetImageMemoryRequirements(mDevice.device(), mImage, &memReq);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memReq.size;
		allocInfo.memoryTypeIndex = mDevice.findMemoryType(memReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		VK_TEST(vkAllocateMemory(mDevice.device(), &allocInfo, CUSTOM_ALLOCATOR, &mImageMemory), "Failed to allocate image memory");
		vkBindImageMemory(mDevice.device(), mImage, mImageMemory, 0);
	}

	void Texture::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
	{
		auto commandBuffer = mDevice.beginSingleTimeCommands();

		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		VkPipelineStageFlags sourceStages;
		VkPipelineStageFlags destinationStage;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			sourceStages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			sourceStages = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else {
			RT_THROW("unsupported layout transition");
		}
		vkCmdPipelineBarrier(commandBuffer, sourceStages, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		mDevice.endSingleTimeCommands(commandBuffer);
	}
}