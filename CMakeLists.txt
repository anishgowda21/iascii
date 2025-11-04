cmake_minimum_required(VERSION 3.10)
project(IAscii)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(OpenCV_DIR "/Users/anishgowda/playground/ThirdParty/opencv-4.12.0/build/resources/lib/cmake/opencv4")

find_package(OpenCV REQUIRED)


include_directories(${OpenCV_INCLUDE_DIRS})


add_executable(iascii src/main.cpp)
target_link_libraries(iascii ${OpenCV_LIBS})
