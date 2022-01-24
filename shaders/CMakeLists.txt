cmake_minimum_required(VERSION 3.18)

find_program(GLSL_VALIDATOR glslangValidator HINTS
                ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE}
            /usr/bin
            /usr/local/bin
            $ENV{VULKAN_SDK}/bin
            $ENV{VULKAN_SDK}/bin32
            $ENV{VULKAN_SDK}/Bin
            $ENV{VULKAN_SDK}/Bin32)

set(SHADERS_SOURCES
    model.frag
    model.vert
    triangle.frag
    triangle.vert)


foreach(SHADER ${SHADERS_SOURCES})
    set(SPIRV_OUT "${PROJECT_BINARY_DIR}/../output/bin/shaders/${SHADER}.spv")
    get_filename_component(FILE_NAME ${SHADER} NAME)
    add_custom_command(
        OUTPUT ${SPIRV_OUT}
        COMMAND ${GLSL_VALIDATOR} -V "${CMAKE_SOURCE_DIR}/shaders/${SHADER}" -o ${SPIRV_OUT}
        DEPENDS ${SHADER}
    )
    list (APPEND SPIRV_BINARY_FILES ${SPIRV_OUT})
endforeach(SHADER)


add_custom_target(shaders DEPENDS ${SPIRV_BINARY_FILES})
