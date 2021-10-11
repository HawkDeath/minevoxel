#include "MineVoxelGame.h"
#include <chrono>
#include "Log.h"

namespace mv {


	void MineVoxelGame::run()
	{

		auto currentTime = std::chrono::high_resolution_clock::now();
		while (!window.shouldClose())
		{
			glfwPollEvents();

			auto newTime = std::chrono::high_resolution_clock::now();
			float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
			currentTime = newTime;

			// draw
			float aspect = renderer.getAspectRatio();

			// set aspect for camera
			if (auto commandBuffer = renderer.beginFrame()) {
				int frameIdx = renderer.getFrameIndex();

				// update UBO; MVP matrix

				renderer.beginSwapChainRenderPass(commandBuffer);
				// render system; call to all objects to draw via vkCmdDraw()

				renderer.endSwapChainRenderPass(commandBuffer);
				renderer.endFrame();
			}

		}
		vkDeviceWaitIdle(device.device());
	}

}