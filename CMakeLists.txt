cmake_minimum_required(VERSION 3.0.0)

set(CMAKE_CXX_STANDARD 17)

# Use the CMakeLists.txt's parent-directory-name for the project's id/name
get_filename_component(PROJECT_ID ${CMAKE_CURRENT_SOURCE_DIR} NAME)
string(REPLACE " " "_" PROJECT_ID ${PROJECT_ID})
project(${PROJECT_ID})

#
# Project Output Paths
#
set(MAINFOLDER ${PROJECT_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${MAINFOLDER}/bin")

#
# Locate Project Prerequisites
#
find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIRS})

#
# Add Build Targets
#
add_executable(${PROJECT_ID} src/main.cpp)
target_link_libraries(${PROJECT_ID} ${CURSES_LIBRARIES})