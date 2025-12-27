cmake_minimum_required(VERSION 3.19)

set(PUBLIC_FILES
    Public/GAPI_BaseHeader.h
    Public/ShaderParameterReflection.h
    Public/SlangHelper.h
)
set(PRIVATE_FILES
    Private/SlangHelper.cpp
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${PUBLIC_FILES} ${PRIVATE_FILES})

# Target settings
add_library(CE-GAPI_Base STATIC
    ${GLOBAL_FILES}
    ${PUBLIC_FILES} ${PRIVATE_FILES}
)
target_include_directories(CE-GAPI_Base
    PUBLIC
        Public
    PRIVATE
        Private
)
target_link_libraries(CE-GAPI_Base
    PRIVATE
        slang
        CE-Base
        CE-Logger
        CE-Platform
        CE-GAPI
        CE-Core
)

# Copy third-party DLibs
add_custom_command(
    TARGET CE-GAPI_Base POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:slang>
        $<TARGET_FILE_DIR:CE-GAPI_Base>
    COMMAND_EXPAND_LISTS
)
