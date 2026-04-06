add_executable(GameEngineRuntime
    main.cpp
    RuntimeApplication.cpp
)

target_link_libraries(GameEngineRuntime PRIVATE GameEngineCore)

if(APPLE)
    set_target_properties(GameEngineRuntime PROPERTIES
        INSTALL_RPATH "@executable_path/../Frameworks"
    )
    find_library(COCOA_LIBRARY Cocoa REQUIRED)
    find_library(IOKIT_LIBRARY IOKit REQUIRED)
    find_library(COREVIDEO_LIBRARY CoreVideo REQUIRED)
    target_link_libraries(GameEngineRuntime PRIVATE
        ${COCOA_LIBRARY}
        ${IOKIT_LIBRARY}
        ${COREVIDEO_LIBRARY}
    )
endif()

if(MSVC)
    target_compile_options(GameEngineRuntime PRIVATE /W3 /MP /utf-8)
    target_compile_definitions(GameEngineRuntime PRIVATE
        _CRT_SECURE_NO_WARNINGS
        NOMINMAX
    )
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang")
    target_compile_options(GameEngineRuntime PRIVATE
        -Wall -Wextra -Wpedantic
        -Wno-deprecated-declarations
    )
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(GameEngineRuntime PRIVATE
        -Wall -Wextra -Wpedantic
    )
endif()

add_custom_command(TARGET GameEngineRuntime POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:GameEngineRuntime>/assets
)

install(TARGETS GameEngineRuntime
    RUNTIME DESTINATION bin
)
