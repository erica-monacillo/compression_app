# filepath: d:\vs code\compression_app\CMakeLists.txt
cmake_minimum_required(VERSION 3.10)

# Set the project name
project(CompressionApp)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add the source files
add_executable(CompressionApp src/main.cpp src/dwt.cpp src/huffman.cpp src/image_io.cpp src/utils.cpp)

# Include directories
include_directories(include)
include_directories(C:/Users/Owner/Downloads/opencv/build/include)

# Link OpenCV libraries
find_package(OpenCV REQUIRED)
target_include_directories(CompressionApp PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(CompressionApp ${OpenCV_LIBS})