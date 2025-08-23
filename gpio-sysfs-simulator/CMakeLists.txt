cmake_minimum_required(VERSION 3.22.0)
project(gpio-sysfs-simulator)
enable_testing()

message(STATUS $ENV{PKG_CONFIG_PATH})
find_package(PkgConfig REQUIRED)
pkg_search_module(FUSE REQUIRED IMPORTED_TARGET fuse3>=3.17)
message(STATUS "FUSE_FOUND: ${FUSE_FOUND}")
message(STATUS "FUSE_VERSION: ${FUSE_VERSION}")
message(STATUS "FUSE_LIBRARIES: ${FUSE_LIBRARIES}")
message(STATUS "FUSE_LIBRARY_DIRS: ${FUSE_LIBRARY_DIRS}")
message(STATUS "FUSE_INCLUDE_DIRS: ${FUSE_INCLUDE_DIRS}")

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

find_package(GTest REQUIRED)
add_executable(test_driver src/test_driver.cpp src/driver.cpp)
target_link_libraries(test_driver GTest::gtest_main pthread)
add_test(NAME test_driver COMMAND test_driver)
