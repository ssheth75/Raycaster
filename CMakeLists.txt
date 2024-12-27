cmake_minimum_required(VERSION 3.10)
project(Raycaster)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add executable
add_executable(Raycaster
    src/main.cpp
    src/Raycaster.cpp
)

# Include directories
target_include_directories(Raycaster PRIVATE include)

# Find and link SFML
find_package(SFML 2.5 COMPONENTS system window graphics REQUIRED)
target_link_libraries(Raycaster sfml-system sfml-window sfml-graphics)
