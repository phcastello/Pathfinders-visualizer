add_library(pathcore STATIC
    src/Version.cpp
    src/Grid.cpp
    src/Dijkstra.cpp
    src/AStar.cpp
    src/MapIO.cpp
)

target_include_directories(pathcore
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

option(PATHVIZ_BUILD_CORE_SMOKE "Build core smoke test" OFF)
if (PATHVIZ_BUILD_CORE_SMOKE)
    add_executable(core_smoke
        src/core_smoke.cpp
    )
    target_link_libraries(core_smoke PRIVATE pathcore)
endif()
