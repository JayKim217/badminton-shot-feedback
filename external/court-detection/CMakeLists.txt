cmake_minimum_required(VERSION 3.16)
project(detect)

set(CMAKE_BUILD_TYPE Release)

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

add_compile_options(
    -Wall
    -Wextra
    -O3
    -DNDEBUG
)

# Use system / Homebrew OpenCV instead of building old OpenCV from source
find_package(OpenCV REQUIRED)

file(GLOB INC_FILES "*.h")
file(GLOB SRC_FILES "*.cpp")

add_executable(detect ${INC_FILES} ${SRC_FILES})

target_compile_features(detect PUBLIC cxx_std_11)

target_include_directories(detect PRIVATE
    ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(detect
    ${OpenCV_LIBS}
)