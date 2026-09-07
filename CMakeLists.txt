cmake_minimum_required(VERSION 3.30)
project(The_Garden_Duel LANGUAGES CXX VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Version
set(RAYLIB_VERSION 6.0)

# Dependencies
find_package(raylib ${RAYLIB_VERSION} QUIET)

if (NOT raylib_FOUND) # If there's none, fetch and build raylib
    message("downloading raylib from git.")
    include(FetchContent)
    FetchContent_Declare(
            raylib
            DOWNLOAD_EXTRACT_TIMESTAMP OFF
            URL https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz
    )
    FetchContent_GetProperties(raylib)
    if (NOT raylib_POPULATED) # Have we downloaded raylib yet?
        set(FETCHCONTENT_QUIET NO)
        FetchContent_MakeAvailable(raylib)
        set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    endif ()
endif ()

add_executable(${PROJECT_NAME}
        headers/Ai.hpp
        headers/Application.hpp
        headers/Common.hpp
        headers/Game.hpp
        headers/Lock.hpp
        headers/Message.hpp
        headers/Particle.hpp
        headers/Plant.hpp
        headers/Player.hpp
        headers/Renderer.hpp
        src/Application.cpp
        src/Game.cpp
        src/Renderer.cpp
        src/main.cpp
)

set(raylib_VERBOSE 1)

if (WIN32)
    target_sources(${PROJECT_NAME} PRIVATE res/app.rc)

    target_link_options(${PROJECT_NAME} PRIVATE
            -static
            -static-libgcc
            -static-libstdc++
    )

    if (CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
        set_property(TARGET ${PROJECT_NAME} PROPERTY WIN32_EXECUTABLE TRUE)
    endif ()
elseif (APPLE)
    set_property(TARGET ${PROJECT_NAME} APPEND PROPERTY MACOSX_BUNDLE TRUE)
elseif (UNIX AND NOT APPLE)
    install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
endif ()

target_include_directories(${PROJECT_NAME} PRIVATE ${raygui_SOURCE_DIR}/src)
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)

if (UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${X11_LIBRARIES})

    # Uncomment if using Wayland
    # target_link_libraries(${PROJECT_NAME} PRIVATE ${Wayland_LIBRARIES})
endif ()

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/res $<TARGET_FILE_DIR:${PROJECT_NAME}>/res
        COMMENT "Copying res/ next to ${PROJECT_NAME}"
)

if (${PLATFORM} STREQUAL "Web")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Os -Wall -s USE_GLFW=3 -s ASSERTIONS=1 -s WASM=1 -s GL_ENABLE_GET_PROC_ADDRESS=1 -s FORCE_FILESYSTEM=1 -s INITIAL_MEMORY=314572800 -s TOTAL_MEMORY=536870912 --preload-file ${CMAKE_CURRENT_SOURCE_DIR}/res@res")
    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".html")
endif ()

# Checks if OSX and links appropriate frameworks (only required on MacOS)
if (APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE "-framework IOKit")
    target_link_libraries(${PROJECT_NAME} PRIVATE "-framework Cocoa")
    target_link_libraries(${PROJECT_NAME} PRIVATE "-framework OpenGL")
endif ()
