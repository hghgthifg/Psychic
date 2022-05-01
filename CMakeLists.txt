# CMakeList.txt: 顶层 CMake 项目文件，在此处执行全局配置
# 并包含子项目。
#
cmake_minimum_required (VERSION 3.8)

project ("Psychic")

add_subdirectory("extern/box2d")
add_subdirectory("extern/glad")
add_subdirectory("extern/glfw")
add_subdirectory("extern/imgui")
add_subdirectory("extern/sajson")
add_subdirectory("Psychic")
