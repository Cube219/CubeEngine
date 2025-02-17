cmake_minimum_required(VERSION 3.19)

include(FetchContent)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui
    GIT_TAG        v1.91.8)
FetchContent_MakeAvailable(imgui)

set(INCLUDE
    ${imgui_SOURCE_DIR}/imconfig.h
    ${imgui_SOURCE_DIR}/imgui.h
    ${imgui_SOURCE_DIR}/imgui_internal.h
    ${imgui_SOURCE_DIR}/imstb_rectpack.h
    ${imgui_SOURCE_DIR}/imstb_textedit.h
    ${imgui_SOURCE_DIR}/imstb_truetype.h
)
set(SOURCE
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
)

if(PLATFORM_WINDOWS)
    set(PLATFORM_INCLUDE
        ${imgui_SOURCE_DIR}/backends/imgui_impl_dx12.h
        ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.h
    )
    set(PLATFORM_SOURCE
        ${imgui_SOURCE_DIR}/backends/imgui_impl_dx12.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp
    )
endif()

# Target settings
add_library(imgui STATIC
    ${SOURCE} ${INCLUDE}
    ${PLATFORM_SOURCE} ${PLATFORM_INCLUDE}
)
target_include_directories(imgui
    PUBLIC
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
)
