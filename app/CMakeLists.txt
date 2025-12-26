set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(pathviz
    src/main.cpp
    src/AppState.cpp
    src/GridView.cpp
    src/LaunchDialog.cpp
    src/MainWindow.cpp
)

# Coloca o binário diretamente em /build (e não /build/app)
set_target_properties(pathviz PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

# Para geradores multi-config (Visual Studio, Ninja Multi-Config)
if(CMAKE_CONFIGURATION_TYPES)
    foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
        string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG_UPPER)
        set_target_properties(pathviz PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER} "${CMAKE_BINARY_DIR}"
        )
    endforeach()
endif()
    

target_link_libraries(pathviz
    PRIVATE
        Qt6::Widgets
        pathcore
)
