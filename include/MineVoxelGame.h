#pragma once

#include "Device.h"
#include "Renderer.h"
#include "Window.h"

namespace mv {
class MineVoxelGame {
  static constexpr auto WIDTH = 1280;
  static constexpr auto HEIGHT = 720;

public:
  MineVoxelGame() = default;
  ~MineVoxelGame() = default;

  void run();

private:
  Window window{"minevoxel", WIDTH, HEIGHT};
  Device device{window};
  Renderer renderer{window, device};
};
} // namespace mv