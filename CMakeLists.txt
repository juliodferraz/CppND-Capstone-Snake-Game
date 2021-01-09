cmake_minimum_required(VERSION 3.7)

add_definitions(-std=c++17)

set(CXX_FLAGS "-Wall")
set(CMAKE_CXX_FLAGS, "${CXX_FLAGS}")

project(SDL2Test)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS} src)

# Find the Tensorflow C++ API.
find_package(TensorflowCC REQUIRED)

add_executable(SnakeGame src/main.cpp src/game.cpp src/controller.cpp src/renderer.cpp src/snake.cpp src/world.cpp src/matrix.cpp)
string(STRIP ${SDL2_LIBRARIES} SDL2_LIBRARIES)
target_link_libraries(SnakeGame ${SDL2_LIBRARIES})

# Link the Tensorflow C++ API.
target_link_libraries(SnakeGame TensorflowCC::TensorflowCC)

# CUDA may also be linked if it is available.
# find_package(CUDA)
# if(CUDA_FOUND)
#   target_link_libraries(SnakeGame ${CUDA_LIBRARIES})
# endif()