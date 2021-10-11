#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <cstdint>
#include <string>
#include <memory>
#include "Input.h"

namespace mv {
	class Window
	{
	public:
		Window(const std::string &title, const std::uint32_t &width, const std::uint32_t &height);
		~Window();

		void createSurface(VkInstance instance, VkSurfaceKHR* surface);

		bool shouldClose() const {
			return static_cast<bool>(glfwWindowShouldClose(mWindow));
		}

		VkExtent2D getExtent2D() const noexcept {
			return { mWidth, mHeight };
		}

		GLFWwindow* window() const {
			return mWindow;
		}

		std::shared_ptr<Input> getInput() const {
			return input;
		}

	private:
		void createWindow();

		static void key_callback(GLFWwindow* win, int key, int scancode, int action, int mods);

	private:
		std::uint32_t mWidth;
		std::uint32_t mHeight;
		std::string mTitle;
		GLFWwindow* mWindow;
		std::shared_ptr<Input> input;
	};

}
