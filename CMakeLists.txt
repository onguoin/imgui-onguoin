cmake_minimum_required(VERSION 3.20)

project(imgui_onguoin VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif ()

if (WIN32)
    set(IMGUI_ONGUOIN_DEFAULT_BUILD_DEMO ON)
else ()
    set(IMGUI_ONGUOIN_DEFAULT_BUILD_DEMO OFF)
endif ()

option(IMGUI_ONGUOIN_FETCH_IMGUI "Download Dear ImGui when IMGUI_DIR is not set." ON)
option(IMGUI_ONGUOIN_BUILD_DEMO "Build the Win32 + DirectX 11 demo application." "${IMGUI_ONGUOIN_DEFAULT_BUILD_DEMO}")

set(IMGUI_DIR "" CACHE PATH "Path to a local Dear ImGui source checkout.")
set(IMGUI_ONGUOIN_IMGUI_GIT_TAG "master" CACHE STRING "Dear ImGui Git tag, branch, or commit used by FetchContent.")

if (NOT TARGET imgui)
    if (NOT IMGUI_DIR)
        if (NOT IMGUI_ONGUOIN_FETCH_IMGUI)
            message(FATAL_ERROR "IMGUI_DIR is required when IMGUI_ONGUOIN_FETCH_IMGUI is OFF.")
        endif ()

        include(FetchContent)
        FetchContent_Declare(
            dear_imgui
            GIT_REPOSITORY https://github.com/ocornut/imgui.git
            GIT_TAG ${IMGUI_ONGUOIN_IMGUI_GIT_TAG}
        )
        FetchContent_MakeAvailable(dear_imgui)
        FetchContent_GetProperties(dear_imgui SOURCE_DIR FETCHED_IMGUI_DIR)
        set(IMGUI_DIR "${FETCHED_IMGUI_DIR}" CACHE PATH "Path to a local Dear ImGui source checkout." FORCE)
    endif ()

    if (NOT EXISTS "${IMGUI_DIR}/imgui.h")
        message(FATAL_ERROR "IMGUI_DIR does not point to a Dear ImGui source tree: ${IMGUI_DIR}")
    endif ()

    add_library(imgui STATIC
        "${IMGUI_DIR}/imgui.cpp"
        "${IMGUI_DIR}/imgui_draw.cpp"
        "${IMGUI_DIR}/imgui_tables.cpp"
        "${IMGUI_DIR}/imgui_widgets.cpp"
    )

    target_include_directories(imgui PUBLIC
        "${IMGUI_DIR}"
    )
endif ()

add_library(imgui_onguoin STATIC
    src/ui/imgui_onguoin.cpp
    src/ui/imgui_onguoin_math.cpp
    src/ui/imgui_onguoin_effects.cpp
    src/ui/imgui_onguoin_background.cpp
    src/ui/imgui_onguoin_fields.cpp
    src/ui/imgui_onguoin_layout.cpp
    src/ui/imgui_onguoin_composite.cpp
    src/ui/imgui_onguoin_surfaces.cpp
    src/ui/imgui_onguoin_widgets.cpp
)

add_library(imgui_onguoin::imgui_onguoin ALIAS imgui_onguoin)

target_include_directories(imgui_onguoin PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>"
)

target_compile_definitions(imgui_onguoin PUBLIC
    WIN32_LEAN_AND_MEAN
    NOMINMAX
)

target_link_libraries(imgui_onguoin PUBLIC
    imgui
)

if (IMGUI_ONGUOIN_BUILD_DEMO)
    if (NOT WIN32)
        message(FATAL_ERROR "The bundled demo is Win32 + DirectX 11 only. Configure with -DIMGUI_ONGUOIN_BUILD_DEMO=OFF on other platforms.")
    endif ()

    if (NOT IMGUI_DIR OR NOT EXISTS "${IMGUI_DIR}/backends/imgui_impl_win32.cpp")
        message(FATAL_ERROR "The demo requires IMGUI_DIR to expose Dear ImGui backends.")
    endif ()

    add_executable(imgui_onguoin_demo WIN32
        src/main.cpp
        "${IMGUI_DIR}/backends/imgui_impl_dx11.cpp"
        "${IMGUI_DIR}/backends/imgui_impl_win32.cpp"
    )

    target_include_directories(imgui_onguoin_demo PRIVATE
        src
        "${IMGUI_DIR}/backends"
    )

    target_compile_definitions(imgui_onguoin_demo PRIVATE
        UNICODE
        _UNICODE
        WIN32_LEAN_AND_MEAN
        NOMINMAX
    )

    target_link_libraries(imgui_onguoin_demo PRIVATE
        imgui_onguoin
        d3d11
        dxgi
        imm32
    )

    set_target_properties(imgui_onguoin_demo PROPERTIES
        OUTPUT_NAME "imgui_onguoin_demo"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/dist"
    )

    foreach(config Debug Release RelWithDebInfo MinSizeRel)
        string(TOUPPER "${config}" config_upper)
        set_target_properties(imgui_onguoin_demo PROPERTIES
            "RUNTIME_OUTPUT_DIRECTORY_${config_upper}" "${CMAKE_BINARY_DIR}/dist"
        )
    endforeach()

    if (MSVC)
        target_compile_options(imgui_onguoin_demo PRIVATE /W4 /EHsc /permissive- /utf-8)
    endif ()
endif ()
