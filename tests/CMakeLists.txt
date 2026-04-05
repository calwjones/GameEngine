include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.6.0
)
FetchContent_MakeAvailable(Catch2)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(Catch)

add_executable(GameEngineTests
    test_collision.cpp
    test_command_history.cpp
    test_entity_manager.cpp
    test_level_io.cpp
    test_gameplay.cpp
    ${CMAKE_SOURCE_DIR}/Editor/CommandHistory.cpp
)

target_include_directories(GameEngineTests PRIVATE
    ${CMAKE_SOURCE_DIR}
)

target_link_libraries(GameEngineTests PRIVATE
    GameEngineCore
    Catch2::Catch2WithMain
)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang")
    target_compile_options(GameEngineTests PRIVATE
        -Wall -Wextra
        -Wno-deprecated-declarations
    )
endif()

catch_discover_tests(GameEngineTests)
