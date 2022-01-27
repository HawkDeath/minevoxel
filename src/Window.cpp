#include "Window.h"
#include "DeviceHelper.h"
#include "Log.h"
#include <stdexcept>

namespace mv {

  Window::Window(const std::string& title, const std::uint32_t& width, const std::uint32_t& height)
    : mTitle{ title }, mWidth{ width }, mHeight{ height }, mWindow{ nullptr }
  {
    glfwSetErrorCallback([](int code, const char* description) -> void {
      ELOG("[GLFW] {}: {}", code, description);
      });

    auto result = glfwInit();
    if (result != GLFW_TRUE) {
      RT_THROW("Failed to initialize GLFW");
    }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    createWindow();
  }

  Window::~Window()
  {
    if (mWindow) {
      glfwDestroyWindow(mWindow);
    }

    glfwTerminate();
  }

  void Window::createSurface(VkInstance instance, VkSurfaceKHR* surface)
  {
    VK_TEST(glfwCreateWindowSurface(instance, mWindow, CUSTOM_ALLOCATOR, surface),
      "Failed to create window surface");
  }

  void Window::createWindow()
  {
    mWindow = glfwCreateWindow(static_cast<int>(mWidth), static_cast<int>(mHeight), mTitle.c_str(), nullptr, nullptr);
    LOG("GLFW {}.{}.{}", GLFW_VERSION_MAJOR, GLFW_VERSION_MINOR, GLFW_VERSION_REVISION);
    glfwSetWindowUserPointer(mWindow, this);
    glfwSetKeyCallback(mWindow, &Window::key_callback);
    glfwSetFramebufferSizeCallback(mWindow, &Window::framebuffer_size_callback);
    input = std::make_shared<Input>();
  }

  void Window::key_callback(GLFWwindow* win, int key, int scancode, int action, int mods)
  {
    UNUSE(scancode);
    UNUSE(mods);
    auto window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(win));
    window->input->keys[key] = action;
  }

  void Window::framebuffer_size_callback(GLFWwindow* win, int width, int height) {
    auto window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(win));
    window->windowWasResized = true;
    window->mWidth = static_cast<std::uint32_t>(width);
    window->mHeight = static_cast<std::uint32_t>(height);
  }
}