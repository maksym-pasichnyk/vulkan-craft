cmake_minimum_required(VERSION 3.16)
project(vcraft)

set(CMAKE_CXX_STANDARD 20)

add_definitions(
    "-DGLFW_INCLUDE_NONE"
    "-DGLFW_INCLUDE_VULKAN"
    "-DGLM_FORCE_DEPTH_ZERO_TO_ONE"

    "-DVULKAN_HPP_DISABLE_IMPLICIT_RESULT_VALUE_CAST"
    "-DVULKAN_HPP_NO_SMART_HANDLE"

    "-DVMA_STATIC_VULKAN_FUNCTIONS"
    "-DVULKAN_HPP_NO_STRUCT_CONSTRUCTORS"
)

include_directories(.)
include_directories(imgui)

function(add_shaders TARGET)
    set(SPIRV_BINARY_FILES)
    foreach(SOURCE_FILE ${ARGN})
        set(SPIRV "${SOURCE_FILE}.spv")
        add_custom_command(
                OUTPUT ${SPIRV}
#                COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_BINARY_DIR}/shaders/"
                COMMAND glslc ${SOURCE_FILE} -o ${SPIRV}
                DEPENDS ${SOURCE_FILE}
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
        list(APPEND SPIRV_BINARY_FILES ${SPIRV})
    endforeach(SOURCE_FILE)

    add_custom_target(${TARGET} DEPENDS ${SPIRV_BINARY_FILES})
endfunction()

add_shaders(shaders
    assets/default.frag
    assets/default.vert
)

add_library(imgui STATIC
    imgui/imconfig.h
    imgui/imgui.h
    imgui/imgui.cpp
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_internal.h
    imgui/imgui_widgets.cpp
    imgui/imstb_rectpack.h
    imgui/imstb_textedit.h
    imgui/imstb_truetype.h)

add_executable(vcraft
    main.cpp
    stb_image.hpp
    Texture.hpp
    main.cpp
    span.hpp
    console.hpp
    stb_image.cpp
    gui.cpp
    gui.hpp
    mem_alloc.hpp
    mem_alloc.cpp
    RenderSystem.hpp
    CommandPool.hpp
    RenderBuffer.hpp
    ResourceManager.hpp
    Buffer.hpp
    Image.hpp
    Texture.hpp
    DescriptorPool.hpp
        Input.hpp
        Window.hpp
        Clock.hpp)

target_link_libraries(vcraft glfw vulkan imgui)
add_dependencies(vcraft shaders)

execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_SOURCE_DIR}/assets ${CMAKE_BINARY_DIR}/assets)