cmake_minimum_required(VERSION 3.8)

#set(VCPKG_ROOT "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/vcpkg")

set(CMAKE_GENERATOR "Ninja")
set(CMAKE_TOOLCHAIN_FILE deps/vcpkg/scripts/buildsystems/vcpkg.cmake)
set(VCPKG_TARGET_TRIPLET "x64-linux")

project(steamtest LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)

find_package(OpenGL REQUIRED)
find_package(imgui CONFIG REQUIRED)

set(DEPS ${CMAKE_CURRENT_SOURCE_DIR}/deps)

add_executable(serial-experiments src/main.cpp)

target_include_directories(
    serial-experiments PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR} 
)

target_link_libraries(serial-experiments PRIVATE imgui::imgui OpenGL::GL)
