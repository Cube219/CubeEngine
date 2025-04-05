cmake_minimum_required(VERSION 3.19)

set(PUBLIC_FILES
    Public/MatrixImpl/MatrixArray.inl
    Public/MatrixImpl/MatrixSSE.inl
    Public/MatrixImpl/MatrixUtilityArray.inl
    Public/MatrixImpl/MatrixUtilitySSE.inl
    Public/VectorImpl/VectorArray.inl
    Public/VectorImpl/VectorSSE.inl
    Public/Allocator.h
    Public/CubeMath.h
    Public/CubeString.h
    Public/Defines.h
    Public/Event.h
    Public/Flags.h
    Public/Format.h
    Public/KeyCode.h
    Public/Matrix.h
    Public/MatrixUtility.h
    Public/Mouse.h
    Public/Types.h
    Public/Vector.h
)

set(PRIVATE_FILES
    Private/CubeString.cpp
    Private/CubeFormat.cpp
)

if(PLATFORM_WINDOWS)
    set(PLATFORM_PUBLIC_FILES
        Public/Windows/WindowsDefines.h
    )
    set(PLATFORM_PRIVATE_FILES
    )
elseif(PLATFORM_MACOS)
    set(PLATFORM_PUBLIC_FILES
        Public/MacOS/MacOSDefines.h
    )
    set(PLATFORM_PRIVATE_FILES
    )
endif()

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${PUBLIC_FILES} ${PRIVATE_FILES} ${PLATFORM_PUBLIC_FILES} ${PLATFORM_PRIVATE_FILES})

# Target settings
add_library(CE-Base STATIC
    ${PUBLIC_FILES} ${PRIVATE_FILES}
)
target_include_directories(CE-Base
    PUBLIC
        Public
    PRIVATE
        Private
)

# Third-party
target_link_libraries(CE-Base
    fmt::fmt
)
