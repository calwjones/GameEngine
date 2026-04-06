# 2D Game Engine with Editor
# Cross-platform: Windows (MSVC/MinGW) and macOS (Clang)

cmake_minimum_required(VERSION 3.20)
project(GameEngine VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

option(BUILD_TESTS "Build unit tests" ON)
option(BUILD_RUNTIME "Build standalone runtime executable" ON)

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
    if(EXISTS "${CMAKE_SOURCE_DIR}/external/SFML/lib/cmake/SFML/SFMLConfig.cmake")
        set(SFML_DIR ${CMAKE_SOURCE_DIR}/external/SFML/lib/cmake/SFML)
        message(STATUS "Using bundled SFML from external/SFML")
    endif()
elseif(APPLE)
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

include(FetchContent)

# 1. RapidJSON
set(RAPIDJSON_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(RAPIDJSON_BUILD_DOC OFF CACHE BOOL "" FORCE)
set(RAPIDJSON_BUILD_TESTS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    rapidjson
    GIT_REPOSITORY https://github.com/Tencent/rapidjson.git
    GIT_TAG master
)
FetchContent_MakeAvailable(rapidjson)

# 2. Dear ImGui
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.8
)
FetchContent_MakeAvailable(imgui)

# 3. ImGui-SFML
set(IMGUI_DIR ${imgui_SOURCE_DIR})
set(IMGUI_SFML_IMGUI_DIR ${imgui_SOURCE_DIR})
set(IMGUI_SFML_FIND_SFML OFF)

FetchContent_Declare(
    imgui_sfml
    GIT_REPOSITORY https://github.com/SFML/imgui-sfml.git
    GIT_TAG v2.6.1
)
FetchContent_MakeAvailable(imgui_sfml)

# ---------------------------------------------------------------------------
# GameEngineCore static library - shared by editor, runtime, and tests
# ---------------------------------------------------------------------------
set(ENGINE_SOURCES
    Engine/Core/GameLoop.cpp
    Engine/Core/Application.cpp
    Engine/Entity/Entity.cpp
    Engine/Entity/EntityManager.cpp
    Engine/Collision/CollisionSystem.cpp
    Engine/Gameplay/GameplaySystem.cpp
    Engine/Level/LevelLoader.cpp
    Engine/Level/LevelGroup.cpp
    Engine/Audio/AudioManager.cpp
    Engine/Physics/PhysicsEngine.cpp
    Engine/Input/InputManager.cpp
    Engine/State/StateManager.cpp
    Engine/Rendering/Camera.cpp
    Engine/Rendering/Renderer.cpp
    Engine/Rendering/TextureManager.cpp
)

add_library(GameEngineCore STATIC ${ENGINE_SOURCES})

target_include_directories(GameEngineCore PUBLIC
    ${CMAKE_SOURCE_DIR}
    ${rapidjson_SOURCE_DIR}/include
)

target_link_libraries(GameEngineCore PUBLIC
    sfml-graphics
    sfml-window
    sfml-system
    OpenGL::GL
)

if(TARGET sfml-audio)
    target_link_libraries(GameEngineCore PUBLIC sfml-audio)
    target_compile_definitions(GameEngineCore PUBLIC HAS_SFML_AUDIO)
    message(STATUS "SFML Audio: Found")
else()
    message(STATUS "SFML Audio: Not found (audio features disabled)")
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang")
    target_compile_options(GameEngineCore PRIVATE
        -Wall -Wextra -Wpedantic
        -Wno-deprecated-declarations
    )
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(GameEngineCore PRIVATE
        -Wall -Wextra -Wpedantic
    )
endif()

# ---------------------------------------------------------------------------
# GameEngine editor executable
# ---------------------------------------------------------------------------
set(EDITOR_SOURCES
    Editor/EditorApplication.cpp
    Editor/ScenePanel.cpp
    Editor/PropertiesPanel.cpp
    Editor/GameViewport.cpp
    Editor/CommandHistory.cpp
    Editor/EntityPalette.cpp
    Editor/SelectionController.cpp
    Editor/LevelIO.cpp
    Editor/PlaySessionController.cpp
    Editor/EntityOps.cpp
    Editor/PopupRenderer.cpp
    Editor/MenuBarRenderer.cpp
    Editor/ToolbarRenderer.cpp
    Editor/OverlayRenderer.cpp
)

if(WIN32)
    add_executable(GameEngine WIN32 main.cpp ${EDITOR_SOURCES})
else()
    add_executable(GameEngine main.cpp ${EDITOR_SOURCES})
endif()

if(APPLE)
    set_target_properties(GameEngine PROPERTIES
        MACOSX_BUNDLE FALSE
        INSTALL_RPATH "@executable_path/../Frameworks"
    )
endif()

target_include_directories(GameEngine PRIVATE
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

target_link_libraries(GameEngine PRIVATE
    GameEngineCore
    ImGui-SFML::ImGui-SFML
)

if(APPLE)
    find_library(COCOA_LIBRARY Cocoa REQUIRED)
    find_library(IOKIT_LIBRARY IOKit REQUIRED)
    find_library(COREVIDEO_LIBRARY CoreVideo REQUIRED)
    target_link_libraries(GameEngine PRIVATE
        ${COCOA_LIBRARY}
        ${IOKIT_LIBRARY}
        ${COREVIDEO_LIBRARY}
    )
elseif(WIN32)
    add_custom_command(TARGET GameEngine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:sfml-graphics>
            $<TARGET_FILE:sfml-window>
            $<TARGET_FILE:sfml-system>
            $<TARGET_FILE_DIR:GameEngine>
    )
    if(TARGET sfml-audio)
        add_custom_command(TARGET GameEngine POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:sfml-audio>
                $<TARGET_FILE_DIR:GameEngine>
        )
    endif()
endif()

add_custom_command(TARGET GameEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:GameEngine>/assets
)

if(MSVC)
    target_compile_options(GameEngine PRIVATE /W3 /MP /utf-8)
    target_compile_definitions(GameEngine PRIVATE
        _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING
        _CRT_SECURE_NO_WARNINGS
        NOMINMAX
    )
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

install(TARGETS GameEngine
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION bin
)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/assets DESTINATION bin)

# ---------------------------------------------------------------------------
# Standalone runtime executable (no editor, no ImGui)
# ---------------------------------------------------------------------------
if(BUILD_RUNTIME)
    add_subdirectory(Runtime)
endif()

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "SFML version: ${SFML_VERSION}")
