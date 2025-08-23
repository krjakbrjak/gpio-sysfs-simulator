cmake_minimum_required(VERSION 3.22.0)
project(superbuild)

include(ExternalProject)

ExternalProject_Add(spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.15.3
    GIT_PROGRESS true
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
)

ExternalProject_Add(fuse3
    GIT_REPOSITORY https://github.com/libfuse/libfuse.git
    GIT_TAG fuse-3.17.4
    GIT_PROGRESS true
    CONFIGURE_COMMAND cd <BINARY_DIR> && meson setup <SOURCE_DIR> --prefix=${CMAKE_INSTALL_PREFIX} -Duseroot=false -Dudevrulesdir= -Dinitscriptdir= -Dexamples=false -Dtests=false --reconfigure
    BUILD_COMMAND cd <BINARY_DIR> && ninja
    INSTALL_COMMAND cd <BINARY_DIR> && ninja install
    TEST_COMMAND ""
)

ExternalProject_Add(gpio_sysfs_simulator
    DEPENDS
        spdlog
        fuse3
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/gpio-sysfs-simulator
    CMAKE_COMMAND ${CMAKE_COMMAND} -E env PKG_CONFIG_PATH=${CMAKE_INSTALL_PREFIX}/lib/x86_64-linux-gnu/pkgconfig -- ${CMAKE_COMMAND}
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
)
