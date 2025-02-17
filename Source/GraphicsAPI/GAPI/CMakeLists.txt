cmake_minimum_required(VERSION 3.19)

set(PUBLIC_FILES
    Public/GAPI_CommandList.h
    Public/GAPI_Fence.h
    Public/GAPI_Viewport.h
    Public/GAPI.h
    Public/GAPIHeader.h
)
set(PRIVATE_FILES
    dummy.cpp
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${PUBLIC_FILES} ${PRIVATE_FILES})

# Target settings
add_library(CE-GAPI STATIC
    ${PUBLIC_FILES} ${PRIVATE_FILES}
)
target_include_directories(CE-GAPI
    PUBLIC
        Public
    PRIVATE
        Private
)
target_link_libraries(CE-GAPI
    PUBLIC
        CE-Base
        CE-Logger
)
