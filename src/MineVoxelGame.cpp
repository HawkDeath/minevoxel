#include "MineVoxelGame.h"
#include <chrono>
#include "Log.h"

#include "systems/TestRenderSystem.h"

namespace mv {
  void MineVoxelGame::run()
  {

    TestRenderSystem renderSystem = { device, renderer.getSwapChainRenderPass(), nullptr };

    auto currentTime = std::chrono::high_resolution_clock::now();
    while (!window.shouldClose())
    {
      glfwPollEvents();

      auto newTime = std::chrono::high_resolution_clock::now();
      float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();

      currentTime = newTime;

      // update
      auto input = window.getInput();
      if (input->getKeyState(GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window.window(), GLFW_TRUE);
      }

      // draw
      float aspect = renderer.getAspectRatio();

      // set aspect for camera
      if (auto commandBuffer = renderer.beginFrame()) {
        int frameIdx = renderer.getFrameIndex();

        FrameInfo frameInfo = {
          commandBuffer,
          nullptr
        };

        // update UBO; MVP matrix

        renderer.beginSwapChainRenderPass(commandBuffer);
        // render system; call to all objects to draw via vkCmdDraw()
        renderSystem.render(commandBuffer);
        renderer.endSwapChainRenderPass(commandBuffer);
        renderer.endFrame();
      }
    }
    vkDeviceWaitIdle(device.device());
  }
}