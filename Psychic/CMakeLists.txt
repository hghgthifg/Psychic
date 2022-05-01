cmake_minimum_required (VERSION 3.8)

set (PSYCHIC_SOURCE_FILES
	"psychic.cpp"
)
set (PSYCHIC_HEAD_FILES
	"psychic.h"
)

add_executable (Psychic ${PSYCHIC_SOURCE_FILES} ${PSYCHIC_HEAD_FILES})

target_include_directories(Psychic PRIVATE ${PSYCHIC_HEAD_FILES})
target_link_libraries(Psychic PUBLIC box2d glfw imgui sajson glad)