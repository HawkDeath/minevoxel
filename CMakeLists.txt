cmake_minimum_required(VERSION 3.10)
project(minevoxel)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# find Vulkan
find_package(Vulkan REQUIRED FATAL_ERROR)


include_directories(./include)

set(MINEVOXEL_HPP
        include/Device.h
        include/DeviceHelper.h
        include/Renderer.h
        include/Pipeline.h
        include/Window.h
        include/SwapChain.h
        include/MineVoxelGame.h)

set(MINEVOXEL_SRC
        src/Device.cpp
        src/DeviceHelper.cpp
        src/Renderer.cpp
        src/Pipeline.cpp
        src/Window.cpp
        src/SwapChain.cpp
        src/MineVoxelGame.cpp)

set(MINEVOXEL_SOURCES main.cpp ${MINEVOXEL_SRC} ${MINEVOXEL_HPP})

add_executable(${PROJECT_NAME} ${MINEVOXEL_SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan::Vulkan)