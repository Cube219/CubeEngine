cmake_minimum_required(VERSION 3.19)

set(PUBLIC_FILES
    Public/GAPI_Metal.h
    Public/MetalHeader.h
)
set(PRIVATE_FILES
    Private/GAPI_Metal.cpp
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
        CE-Base
        CE-Logger
        CE-GAPI
        CE-Core
)
target_compile_definitions(CE-GAPI_Metal
    PRIVATE
        CUBE_MODULE_GAPI_METAL
)
