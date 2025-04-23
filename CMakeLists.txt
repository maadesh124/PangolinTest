cmake_minimum_required(VERSION 3.1)
project(TestPangolin)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Pangolin REQUIRED)
include_directories(
    ${Pangolin_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/../Pangolin/tools/ModelViewer
    ${PROJECT_SOURCE_DIR}/../Pangolin/components/tinyobj/include/tinyobj


)
add_executable(test_window Object.cpp 
main.cpp
)

target_link_libraries(test_window ${Pangolin_LIBRARIES})
