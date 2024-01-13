cmake_minimum_required(VERSION 3.22.0)
project(gpio-sysfs-simulator)

find_package(PkgConfig REQUIRED)
pkg_search_module(FUSE REQUIRED IMPORTED_TARGET fuse3)
find_package(spdlog REQUIRED)

set(CMAKE_CXX_STANDARD 17)
add_executable(${PROJECT_NAME}
    src/main.cpp
    src/driver.cpp
)
target_link_libraries(${PROJECT_NAME} PUBLIC
    PkgConfig::FUSE
    spdlog::spdlog
)
target_compile_definitions(${PROJECT_NAME} PUBLIC _FILE_OFFSET_BITS=64)

install(TARGETS ${PROJECT_NAME})
