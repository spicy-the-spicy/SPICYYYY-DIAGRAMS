cmake_minimum_required(VERSION 3.16)
project(FilmGrainPlugin VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find threads for multi-threaded processing
find_package(Threads REQUIRED)

# Adobe Premiere Pro SDK path (set via environment or command line)
# Example: cmake -DADOBE_PREMIERE_SDK_PATH=/path/to/sdk ..
set(ADOBE_PREMIERE_SDK_PATH "" CACHE PATH "Path to Adobe Premiere Pro SDK")

# Plugin source files
set(PLUGIN_SOURCES
    src/PluginMain.cpp
    src/GrainEngine.cpp
    src/Presets.cpp
)

# Plugin header files
set(PLUGIN_HEADERS
    include/GrainEngine.h
    include/Presets.h
    include/PluginConfig.h
)

# Create shared library (plugin)
add_library(FilmGrainPlugin SHARED
    ${PLUGIN_SOURCES}
    ${PLUGIN_HEADERS}
)

# Include directories
target_include_directories(FilmGrainPlugin PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link with threads
target_link_libraries(FilmGrainPlugin PRIVATE Threads::Threads)

# If Adobe SDK path is provided, include it
if(ADOBE_PREMIERE_SDK_PATH)
    target_include_directories(FilmGrainPlugin PRIVATE
        ${ADOBE_PREMIERE_SDK_PATH}/Examples/Headers
        ${ADOBE_PREMIERE_SDK_PATH}/Examples/Headers/SP
        ${ADOBE_PREMIERE_SDK_PATH}/Examples/Headers/PrSDKAESupport
    )
    target_compile_definitions(FilmGrainPlugin PRIVATE
        PREMIERE_SDK_AVAILABLE=1
    )
else()
    message(STATUS "Adobe Premiere SDK not specified. Building in standalone mode for testing.")
    target_compile_definitions(FilmGrainPlugin PRIVATE
        PREMIERE_SDK_AVAILABLE=0
    )
endif()

# Platform-specific settings
if(WIN32)
    # Windows plugin extension
    set_target_properties(FilmGrainPlugin PROPERTIES
        SUFFIX ".prm"
        PREFIX ""
    )
    target_compile_definitions(FilmGrainPlugin PRIVATE
        _WINDOWS
        WIN32_LEAN_AND_MEAN
    )
elseif(APPLE)
    # macOS plugin extension
    set_target_properties(FilmGrainPlugin PROPERTIES
        SUFFIX ".plugin"
        PREFIX ""
        BUNDLE TRUE
        BUNDLE_EXTENSION "plugin"
    )
    target_compile_definitions(FilmGrainPlugin PRIVATE
        __APPLE__
    )
else()
    # Linux (for development/testing)
    set_target_properties(FilmGrainPlugin PROPERTIES
        SUFFIX ".so"
        PREFIX "lib"
    )
endif()

# Compiler flags
if(MSVC)
    target_compile_options(FilmGrainPlugin PRIVATE
        /W4
        /O2
    )
else()
    target_compile_options(FilmGrainPlugin PRIVATE
        -Wall
        -Wextra
        -O3
        -march=native
    )
endif()

# Installation
install(TARGETS FilmGrainPlugin
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

# Standalone test executable (only when SDK not available)
if(NOT ADOBE_PREMIERE_SDK_PATH)
    add_executable(FilmGrainTest
        ${PLUGIN_SOURCES}
        ${PLUGIN_HEADERS}
    )
    target_include_directories(FilmGrainTest PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    target_link_libraries(FilmGrainTest PRIVATE Threads::Threads)
    target_compile_definitions(FilmGrainTest PRIVATE
        PREMIERE_SDK_AVAILABLE=0
    )
    if(NOT MSVC)
        target_compile_options(FilmGrainTest PRIVATE
            -Wall
            -Wextra
            -O3
            -march=native
        )
    endif()
endif()

# Copy resources to build directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/resources
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# Print configuration summary
message(STATUS "=== Film Grain Plugin Configuration ===")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
if(ADOBE_PREMIERE_SDK_PATH)
    message(STATUS "Adobe SDK: ${ADOBE_PREMIERE_SDK_PATH}")
else()
    message(STATUS "Adobe SDK: Not specified (standalone mode)")
endif()
