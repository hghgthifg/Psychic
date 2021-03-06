﻿cmake_minimum_required (VERSION 3.8)

set (PSYCHIC_SOURCE_FILES
	"Psychic.cpp"
    "imgui_impl_glfw.cpp"
    "imgui_impl_opengl3.cpp"
    "draw.cpp"
    "settings.cpp"
    "scene.cpp")
set (PSYCHIC_HEAD_FILES
    "imgui_impl_glfw.h"
    "imgui_impl_opengl3.h"
    "draw.h"
    "settings.h"
    "scene.h"
    "stb_image.h")

add_executable (Psychic ${PSYCHIC_SOURCE_FILES} ${PSYCHIC_HEAD_FILES})

target_include_directories(Psychic PRIVATE ${PSYCHIC_HEAD_FILES})
target_link_libraries(Psychic PUBLIC box2d glfw imgui sajson glad)

add_custom_command(
        TARGET Psychic POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_CURRENT_SOURCE_DIR}/resource/
                ${CMAKE_CURRENT_BINARY_DIR}/resource/)