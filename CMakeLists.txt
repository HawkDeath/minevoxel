cmake_minimum_required(VERSION 3.18)
project(minevoxel)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/output")
file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/output/bin")
file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/output/lib")
file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/output/bin/shaders")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/output/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/output/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/output/lib")

# find Vulkan
find_package(Vulkan REQUIRED FATAL_ERROR)
find_package(Vulkan COMPONENTS glslc)

# glfw
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)
add_subdirectory(deps/glfw)

# glm
add_library(glm INTERFACE)
set(GLM_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps/glm)
target_sources(glm INTERFACE ${GLM_DIR}/glm/glm.hpp)
target_include_directories(glm INTERFACE ${GLM_DIR})

target_compile_definitions(glm INTERFACE
        GLM_FORCE_SWIZZLE
        GLM_FORCE_RADIANS
        GLM_FORCE_CTOR_INIT
        GLM_FORCE_DEPTH_ZERO_TO_ONE
        GLM_ENABLE_EXPERIMENTAL)
if(NOT CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_definitions(glm INTERFACE GLM_FORCE_CXX14)
endif()

# spdlog
add_subdirectory(deps/spdlog)

include_directories(./include)

set(MINEVOXEL_HPP
        include/systems/ModelTestRenderSystem.h
        include/systems/TestRenderSystem.h
        include/Buffer.h
        include/Camera.h
        include/Device.h
        include/DeviceHelper.h
        include/Descriptors.h
        include/Model.h
        include/Renderer.h
        include/Pipeline.h
        include/Window.h
        include/SwapChain.h
        include/MineVoxelGame.h)

set(MINEVOXEL_SRC
        src/systems/ModelTestRenderSystem.cpp
        src/systems/TestRenderSystem.cpp
        src/Buffer.cpp
        src/Camera.cpp
        src/Device.cpp
        src/DeviceHelper.cpp
        src/Descriptors.cpp
        src/Model.cpp
        src/Renderer.cpp
        src/Pipeline.cpp
        src/Window.cpp
        src/SwapChain.cpp
        src/MineVoxelGame.cpp)

set(MINEVOXEL_SOURCES main.cpp ${MINEVOXEL_SRC} ${MINEVOXEL_HPP})
add_subdirectory(shaders)


add_executable(${PROJECT_NAME} ${MINEVOXEL_SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog Vulkan::Vulkan glfw glm)

add_dependencies(${PROJECT_NAME} shaders)
