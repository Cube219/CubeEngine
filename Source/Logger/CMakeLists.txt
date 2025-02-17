cmake_minimum_required(VERSION 3.19)

set(PUBLIC_FILES
    Public/Checker.h
    Public/Logger.h
    Public/LoggerHeader.h
)
set(PRIVATE_FILES
    Private/Checker.cpp
    Private/Logger.cpp
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${PUBLIC_FILES} ${PRIVATE_FILES})

# Target settings
add_library(CE-Logger SHARED ${PUBLIC_FILES} ${PRIVATE_FILES})
target_include_directories(CE-Logger
    PUBLIC
        Public
    PRIVATE
        Private
)
target_link_libraries(CE-Logger
    PRIVATE
        CE-Base
)
target_compile_definitions(CE-Logger
    PRIVATE
        CUBE_MODULE_LOGGER
)
