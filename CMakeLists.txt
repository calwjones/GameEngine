# 2D Game Engine with Editor
# Cross-platform: Windows (MSVC/MinGW) and macOS (Clang)

cmake_minimum_required(VERSION 3.20)
project(GameEngine VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Platform detection
if(WIN32)
    message(STATUS "Configuring for Windows")
elseif(APPLE)
    message(STATUS "Configuring for macOS")
else()
    message(STATUS "Configuring for Unix/Linux")
endif()

# SFML setup - platform-specific paths
if(WIN32)
    # Windows: Use bundled SFML from external directory
    if(EXISTS "${CMAKE_SOURCE_DIR}/external/SFML/lib/cmake/SFML/SFMLConfig.cmake")
        set(SFML_DIR ${CMAKE_SOURCE_DIR}/external/SFML/lib/cmake/SFML)
        message(STATUS "Using bundled SFML from external/SFML")
    endif()
elseif(APPLE)
    # macOS: Check Homebrew locations
    if(EXISTS "/opt/homebrew/Cellar/sfml@2")
        file(GLOB SFML_HOMEBREW_PATHS "/opt/homebrew/Cellar/sfml@2/*/lib/cmake/SFML")
        if(SFML_HOMEBREW_PATHS)
            list(GET SFML_HOMEBREW_PATHS 0 SFML_DIR)
            message(STATUS "Using Homebrew SFML from ${SFML_DIR}")
        endif()
    elseif(EXISTS "/usr/local/Cellar/sfml@2")
        file(GLOB SFML_HOMEBREW_PATHS "/usr/local/Cellar/sfml@2/*/lib/cmake/SFML")
        if(SFML_HOMEBREW_PATHS)
            list(GET SFML_HOMEBREW_PATHS 0 SFML_DIR)
            message(STATUS "Using Homebrew SFML from ${SFML_DIR}")
        endif()
    endif()
endif()

# Dependencies - support SFML 2.5 or 2.6 (audio optional)
find_package(SFML 2.5 COMPONENTS graphics window system audio QUIET)
if(NOT SFML_FOUND)
    find_package(SFML 2.6 COMPONENTS graphics window system audio QUIET)
endif()
if(NOT SFML_FOUND)
    find_package(SFML 2.5 COMPONENTS graphics window system QUIET)
    if(NOT SFML_FOUND)
        find_package(SFML 2.6 COMPONENTS graphics window system REQUIRED)
    endif()
endif()
find_package(OpenGL REQUIRED)

set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/external/imgui)
set(IMGUI_SFML_DIR ${CMAKE_SOURCE_DIR}/external/imgui-sfml)
set(RAPIDJSON_DIR ${CMAKE_SOURCE_DIR}/external/rapidjson)

if(NOT EXISTS "${IMGUI_DIR}/imgui.cpp")
    message(FATAL_ERROR "ImGui not found in ${IMGUI_DIR}")
endif()

if(NOT EXISTS "${IMGUI_SFML_DIR}/imgui-SFML.cpp")
    message(FATAL_ERROR "ImGui-SFML not found in ${IMGUI_SFML_DIR}")
endif()

if(NOT EXISTS "${RAPIDJSON_DIR}/include/rapidjson/document.h")
    message(FATAL_ERROR "RapidJSON not found in ${RAPIDJSON_DIR}")
endif()

set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
)

set(IMGUI_SFML_SOURCES
    ${IMGUI_SFML_DIR}/imgui-SFML.cpp
)

set(ENGINE_SOURCES
    Engine/Core/GameLoop.cpp
    Engine/Core/Application.cpp
    Engine/Entity/Entity.cpp
    Engine/Entity/EntityManager.cpp
    Engine/Collision/CollisionSystem.cpp
    Engine/Gameplay/GameplaySystem.cpp
    Engine/Level/LevelLoader.cpp
    Engine/Level/LevelGroup.cpp
)

set(EDITOR_SOURCES
    Editor/EditorApplication.cpp
    Editor/ScenePanel.cpp
    Editor/PropertiesPanel.cpp
    Editor/GameViewport.cpp
    Editor/CommandHistory.cpp
    Editor/EntityPalette.cpp
)

# Create executable (WIN32 flag hides console window on Windows)
if(WIN32)
    add_executable(GameEngine WIN32
        main.cpp
        ${ENGINE_SOURCES}
        ${EDITOR_SOURCES}
        ${IMGUI_SOURCES}
        ${IMGUI_SFML_SOURCES}
    )
else()
    add_executable(GameEngine
        main.cpp
        ${ENGINE_SOURCES}
        ${EDITOR_SOURCES}
        ${IMGUI_SOURCES}
        ${IMGUI_SFML_SOURCES}
    )
endif()

# macOS: Create app bundle structure
if(APPLE)
    set_target_properties(GameEngine PROPERTIES
        MACOSX_BUNDLE FALSE
        INSTALL_RPATH "@executable_path/../Frameworks"
    )
endif()

target_include_directories(GameEngine PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${IMGUI_DIR}
    ${IMGUI_SFML_DIR}
    ${RAPIDJSON_DIR}/include
)

# Platform-specific linking
if(APPLE)
    # macOS: Link against frameworks
    find_library(COCOA_LIBRARY Cocoa REQUIRED)
    find_library(IOKIT_LIBRARY IOKit REQUIRED)
    find_library(COREVIDEO_LIBRARY CoreVideo REQUIRED)
    target_link_libraries(GameEngine PRIVATE
        sfml-graphics
        sfml-window
        sfml-system
        OpenGL::GL
        ${COCOA_LIBRARY}
        ${IOKIT_LIBRARY}
        ${COREVIDEO_LIBRARY}
    )
    if(TARGET sfml-audio)
        target_link_libraries(GameEngine PRIVATE sfml-audio)
    endif()
elseif(WIN32)
    target_link_libraries(GameEngine PRIVATE
        sfml-graphics
        sfml-window
        sfml-system
        OpenGL::GL
    )
    if(TARGET sfml-audio)
        target_link_libraries(GameEngine PRIVATE sfml-audio)
    endif()
    # Copy SFML DLLs to output directory
    add_custom_command(TARGET GameEngine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:sfml-graphics>
            $<TARGET_FILE:sfml-window>
            $<TARGET_FILE:sfml-system>
            $<TARGET_FILE_DIR:GameEngine>
    )
    # Also copy OpenAL DLL if SFML audio is used
    if(TARGET sfml-audio)
        add_custom_command(TARGET GameEngine POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:sfml-audio>
                $<TARGET_FILE_DIR:GameEngine>
        )
    endif()
else()
    # Linux/Unix
    target_link_libraries(GameEngine PRIVATE
        sfml-graphics
        sfml-window
        sfml-system
        OpenGL::GL
    )
    if(TARGET sfml-audio)
        target_link_libraries(GameEngine PRIVATE sfml-audio)
    endif()
endif()

# Enable audio features if sfml-audio is available
if(TARGET sfml-audio)
    target_compile_definitions(GameEngine PRIVATE HAS_SFML_AUDIO)
    message(STATUS "SFML Audio: Found")
else()
    message(STATUS "SFML Audio: Not found (audio features disabled)")
endif()

add_custom_command(TARGET GameEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:GameEngine>/assets
)

# Compiler-specific options
if(MSVC)
    target_compile_options(GameEngine PRIVATE
        /W3
        /MP           # Multi-processor compilation
        /utf-8        # UTF-8 source files
    )
    target_compile_definitions(GameEngine PRIVATE
        _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING
        _CRT_SECURE_NO_WARNINGS
        NOMINMAX      # Prevent Windows.h min/max macros
    )
    # Use WinMain entry point for GUI app
    set_target_properties(GameEngine PROPERTIES
        LINK_FLAGS "/ENTRY:mainCRTStartup"
    )
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang")
    target_compile_options(GameEngine PRIVATE
        -Wall -Wextra -Wpedantic
        -Wno-deprecated-declarations
    )
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(GameEngine PRIVATE
        -Wall -Wextra -Wpedantic
    )
endif()

# Install targets
install(TARGETS GameEngine
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION bin
)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/assets DESTINATION bin)
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "SFML version: ${SFML_VERSION}")
