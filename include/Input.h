#pragma once

#include <GLFW/glfw3.h>
#include <array>
#include <cassert>

namespace mv {
class Window;
class Input {
  friend class Window;

public:
  Input() = default;
  ~Input() = default;

  int getKeyState(const int &idx) noexcept {
    assert(idx >= keys.size());
    return keys[idx];
  }

private:
  std::array<int, GLFW_KEY_LAST> keys;
};
} // namespace mv
