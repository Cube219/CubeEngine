cmake_minimum_required(VERSION 3.19)

set(PUBLIC_FILES
    Public/GAPI_Buffer.h
    Public/GAPI_CommandList.h
    Public/GAPI_Fence.h
    Public/GAPI_Sampler.h
    Public/GAPI_Pipeline.h
    Public/GAPI_Resource.h
    Public/GAPI_Shader.h
    Public/GAPI_ShaderParameter.h
    Public/GAPI_SwapChain.h
    Public/GAPI_Texture.h
    Public/GAPI_Timestamp.h
    Public/GAPI.h
    Public/GAPIHeader.h
    Public/GAPITypes.h
)
set(PRIVATE_FILES
    dummy.cpp
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${PUBLIC_FILES} ${PRIVATE_FILES})

# Target settings
add_library(CE-GAPI STATIC
    ${GLOBAL_FILES}
    ${PUBLIC_FILES} ${PRIVATE_FILES}
)
target_include_directories(CE-GAPI
    PUBLIC
        Public
    PRIVATE
        Private
)
target_link_libraries(CE-GAPI
    PRIVATE
        CE-Base
        CE-Logger
)
