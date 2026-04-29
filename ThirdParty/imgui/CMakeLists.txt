cmake_minimum_required(VERSION 3.19)

include(FetchContent)

message(STATUS "CubeEngine: Fetching imgui/imGuIZMO/implot...")

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui
        # Commit that support changing sampler in Metal.
        # See https://github.com/ocornut/imgui/issues/9378.
    GIT_TAG        dee9b15bbec8c8a7120e4dfddf73b35ae60947f1
)
FetchContent_Declare(
    imguizmoquat
    GIT_REPOSITORY https://github.com/BrutPitt/imGuIZMO.quat
    GIT_TAG        f0328f070263c4e7f945e912319457db8e883ca2 # v4.0
)
FetchContent_Declare(
    implot
    GIT_REPOSITORY https://github.com/epezent/implot
    GIT_TAG        524f9fcd48d76c13fdf94c5ffbba8787a1ff7e39 # v1.0
)
FetchContent_MakeAvailable(imgui imguizmoquat implot)

set(INCLUDE
    ${imgui_SOURCE_DIR}/imconfig.h
    ${imgui_SOURCE_DIR}/imgui.h
    ${imgui_SOURCE_DIR}/imgui_internal.h
    ${imgui_SOURCE_DIR}/imstb_rectpack.h
    ${imgui_SOURCE_DIR}/imstb_textedit.h
    ${imgui_SOURCE_DIR}/imstb_truetype.h
    ${imguizmoquat_SOURCE_DIR}/imguizmo_quat/imGuIZMOquat.h
    ${implot_SOURCE_DIR}/implot.h
    ${implot_SOURCE_DIR}/implot_internal.h
)
set(SOURCE
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imguizmoquat_SOURCE_DIR}/imguizmo_quat/imGuIZMOquat.cpp
    ${implot_SOURCE_DIR}/implot.cpp
    ${implot_SOURCE_DIR}/implot_items.cpp
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
if(PLATFORM_MACOS)
    set(PLATFORM_INCLUDE
        ${imgui_SOURCE_DIR}/backends/imgui_impl_metal.h
        ${imgui_SOURCE_DIR}/backends/imgui_impl_osx.h
    )
    set(PLATFORM_SOURCE
        ${imgui_SOURCE_DIR}/backends/imgui_impl_metal.mm
        ${imgui_SOURCE_DIR}/backends/imgui_impl_osx.mm
    )
    # Enable Automatic Reference Counting (ARC) for Objective-C++ files (.mm)
    set_source_files_properties(${PLATFORM_SOURCE}
        PROPERTIES
            COMPILE_FLAGS "-fobjc-arc"
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
        ${imguizmoquat_SOURCE_DIR}
        ${implot_SOURCE_DIR}
)
target_compile_definitions(imgui
    PUBLIC
        IMGUIZMO_IMGUI_FOLDER= # Empty
)
