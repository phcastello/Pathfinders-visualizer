add_library(pathcore STATIC
    src/Version.cpp
)

target_include_directories(pathcore
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
