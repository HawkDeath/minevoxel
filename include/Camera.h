#pragma once

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace mv {
class Camera {
public:
  Camera() {
    projectionMatrix = glm::mat4(1.0f);
    viewMatrix = glm::mat4(1.0f);
  }

  glm::mat4 getViewMatrix() const { return viewMatrix; }

private:
  glm::mat4 projectionMatrix;
  glm::mat4 viewMatrix;
};
} // namespace mv
