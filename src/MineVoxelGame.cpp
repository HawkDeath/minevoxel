#include "MineVoxelGame.h"
#include "Log.h"
#include <chrono>

#include "Descriptors.h"

#include "Model.h"
#include "Texture.h"
#include "systems/ModelTestRenderSystem.h"
#include "systems/TestRenderSystem.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#ifndef RESOURCES_PATH
#define RESOURCES_PATH "./"
#endif

namespace mv {
  struct UniformBufferObj {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
  };

  void MineVoxelGame::run() {

    std::string cubeModelPath = RESOURCES_PATH + std::string("/room.obj");
    std::string modelTexture = RESOURCES_PATH + std::string("/viking_room.png");

    ModelLoader loader;
    loader.load(cubeModelPath);

    Texture texture = {
      device,
      modelTexture,
      VK_FORMAT_R8G8B8A8_SRGB
    };


    std::vector<std::unique_ptr<Buffer>> uboBuffers(
      SwapChain::MAX_FRAME_IN_FLIGHT);
    auto sizeOfBuffer = static_cast<VkDeviceSize>(sizeof(UniformBufferObj));

    for (int i = 0; i < uboBuffers.size(); i++) {
      uboBuffers[i] = std::make_unique<Buffer>(
        device, sizeOfBuffer, 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
      uboBuffers[i]->map(sizeOfBuffer);
    }

    std::unique_ptr<DescriptorSetLayout> globalDescriptorSetLayout =
      std::make_unique<DescriptorSetLayout>(device);
    globalDescriptorSetLayout->addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      VK_SHADER_STAGE_ALL_GRAPHICS);
    globalDescriptorSetLayout->addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS);
    globalDescriptorSetLayout->createDescriptorSetLayout();

    std::unique_ptr<DescriptorPool> globalPool =
      std::make_unique<DescriptorPool>(device);
    globalPool->setMaxSets(SwapChain::MAX_FRAME_IN_FLIGHT);
    globalPool->addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      SwapChain::MAX_FRAME_IN_FLIGHT);
    globalPool->createDescriptorPool();

    std::vector<VkDescriptorSet> globalDescriptorSets(
      SwapChain::MAX_FRAME_IN_FLIGHT);
    for (int i = 0; i < globalDescriptorSets.size(); i++) {
      auto bufferInfo = uboBuffers[i]->descriptorInfo(sizeOfBuffer);
      VkDescriptorImageInfo imgInfo = {};
      imgInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      imgInfo.imageView = texture.getImageView();
      imgInfo.sampler = texture.getSampler();

      DescriptorWriter writer = { *globalDescriptorSetLayout, *globalPool };
      writer.writeBuffer(0, &bufferInfo);
      writer.writeImage(1, &imgInfo);
      writer.build(globalDescriptorSets[i]);
    }

    ModelTestRenderSystem renderSystem = {
        device, renderer.getSwapChainRenderPass(),
        globalDescriptorSetLayout->getDescriptorSetLayout() };

    std::vector<std::unique_ptr<Model>> models;
    models.push_back(std::make_unique<Model>(device, loader));

    auto currentTime = std::chrono::high_resolution_clock::now();
    auto input = window.getInput();

    glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    UniformBufferObj ubo = {};
    ubo.model = glm::mat4(1.0f);
    while (!window.shouldClose()) {
      glfwPollEvents();

      auto newTime = std::chrono::high_resolution_clock::now();
      auto frameTime = std::chrono::duration<float, std::chrono::seconds::period>(
        newTime - currentTime)
        .count();

      currentTime = newTime;

      // update
      if (input->getKeyState(GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window.window(), GLFW_TRUE);
      }

      auto step = 5.f;
      auto velocity = step * frameTime;
      if (input->getKeyState(GLFW_KEY_W)) {
        cameraPos.z -= velocity;
      }
      if (input->getKeyState(GLFW_KEY_S)) {
        cameraPos.z += velocity;
      }
      if (input->getKeyState(GLFW_KEY_A)) {
        cameraPos.x -= velocity;
      }
      if (input->getKeyState(GLFW_KEY_D)) {
        cameraPos.x += velocity;
      }

      // draw
      auto aspect = renderer.getAspectRatio();
      auto frameIdx = renderer.getFrameIndex();

      ubo.projection =
        glm::perspective(glm::radians(90.0f), (float)aspect, 0.1f, 100.0f);
      ubo.view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
      ubo.model = glm::rotate(ubo.model, glm::radians(frameTime * -45.0f),
        glm::vec3(0.0f, 1.0f, 0.0f));


      // set aspect for camera
      if (auto commandBuffer = renderer.beginFrame()) {
        int frameIdx = renderer.getFrameIndex();

        FrameInfo frameInfo = { commandBuffer, globalDescriptorSets[frameIdx],
                               models };

        // update UBO; MVP matrix

        uboBuffers[frameIdx]->writeToBuffer(&ubo, sizeof(ubo));
        uboBuffers[frameIdx]->flush(sizeof(ubo));

        renderer.beginSwapChainRenderPass(frameInfo.commandBuffer);
        // render system; call to all objects to draw via vkCmdDraw()
        renderSystem.render(frameInfo);
        renderer.endSwapChainRenderPass(frameInfo.commandBuffer);
        renderer.endFrame();
      }
    }
    vkDeviceWaitIdle(device.device());
  }
} // namespace mv