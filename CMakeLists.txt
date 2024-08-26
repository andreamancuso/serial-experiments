cmake_minimum_required(VERSION 3.8)

set(CMAKE_GENERATOR "Ninja")
set(CMAKE_TOOLCHAIN_FILE deps/vcpkg/scripts/buildsystems/vcpkg.cmake)
set(VCPKG_TARGET_TRIPLET "x64-linux")

project(serial-experiments LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)

find_package(OpenGL REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(CSerialPort QUIET)
find_package(LibCommsdsl CONFIG REQUIRED)

set(DEPS ${CMAKE_CURRENT_SOURCE_DIR}/deps)

add_executable(serial-experiments
        src/main.cpp
        src/session.cpp
        src/shared.cpp
)
include_directories(${CSerialPort_INCLUDE_DIR})

target_include_directories(
    serial-experiments PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${DEPS}/comms/include
    ${DEPS}/cc.ublox.generated/include
)

target_link_libraries(serial-experiments PRIVATE imgui::imgui OpenGL::GL glfw cc::commsdsl ${CSerialPort_LIBRARY})
