cmake_minimum_required(VERSION 3.19)

set(PUBLIC_FILES
    Public/GAPI_Metal.h
    Public/GAPI_MetalBuffer.h
    Public/GAPI_MetalCommandList.h
    Public/GAPI_MetalFence.h
    Public/GAPI_MetalPipeline.h
    Public/GAPI_MetalResource.h
    Public/GAPI_MetalShader.h
    Public/GAPI_MetalShaderVariable.h
    Public/GAPI_MetalViewport.h
    Public/MetalHeader.h
)
set(PRIVATE_FILES
    Private/GAPI_Metal.mm
    Private/GAPI_MetalViewport.mm
    Private/MetalDevice.h
    Private/MetalDevice.mm
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${PUBLIC_FILES} ${PRIVATE_FILES})

# Target settings
add_library(CE-GAPI_Metal SHARED
    ${PUBLIC_FILES} ${PRIVATE_FILES}
)
target_include_directories(CE-GAPI_Metal
    PUBLIC
        Public
    PRIVATE
        Private
)
target_link_libraries(CE-GAPI_Metal
    PRIVATE
        "-framework AppKit"
        "-framework GameController"
        "-framework Metal"
        "-framework MetalKit"
        imgui
        CE-Base
        CE-Logger
        CE-Platform
        CE-GAPI
        CE-Core
)
target_compile_definitions(CE-GAPI_Metal
    PRIVATE
        CUBE_MODULE_GAPI_METAL
)
# Metal validation
# if(DEBUG_MODE)
# target_compile_definitions(CE-GAPI_Metal
#     PRIVATE
#         METAL_DEVICE_WRAPPER_TYPE=1
# )
# endif()
if(DEBUG_MODE)
target_compile_definitions(CE-GAPI_Metal
    PRIVATE
        MTL_CAPTURE_ENABLED=1
)
endif()

list(APPEND G_ENABLED_GAPI_MODULES "CE-GAPI_Metal")
set(G_ENABLED_GAPI_MODULES "${G_ENABLED_GAPI_MODULES}" CACHE INTERNAL "g_enabled_gapi_modules")
