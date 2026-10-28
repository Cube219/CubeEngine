cmake_minimum_required(VERSION 3.19)

set(PUBLIC_FILES
    Public/Allocator/AllocatorUtility.h
    Public/Allocator/FrameAllocator.h
    Public/MatrixImpl/MatrixArray.inl
    Public/MatrixImpl/MatrixAVX2.inl
    Public/MatrixImpl/MatrixSSE.inl
    Public/MatrixImpl/MatrixUtilityArray.inl
    Public/MatrixImpl/MatrixUtilityAVX2.inl
    Public/MatrixImpl/MatrixUtilitySSE.inl
    Public/Platform/DLib.h
    Public/Platform/FileSystem.h
    Public/Platform/Platform.h
    Public/Platform/PlatformDebug.h
    Public/QuaternionImpl/QuaternionArray.inl
    Public/QuaternionImpl/QuaternionAVX2.inl
    Public/QuaternionImpl/QuaternionNEON.inl
    Public/QuaternionImpl/QuaternionSSE.inl
    Public/VectorImpl/VectorArray.inl
    Public/VectorImpl/VectorAVX2.inl
    Public/VectorImpl/VectorSSE.inl
    Public/Async.h
    Public/Blob.h
    Public/Checker.h
    Public/CoreHeader.h
    Public/CubeMath.h
    Public/CubeString.h
    Public/Defines.h
    Public/Event.h
    Public/Flags.h
    Public/Format.h
    Public/KeyCode.h
    Public/Logger.h
    Public/Matrix.h
    Public/MatrixUtility.h
    Public/Mouse.h
    Public/Quaternion.h
    Public/Types.h
    Public/Vector.h
)

set(PRIVATE_FILES
    Private/Allocator/FrameAllocator.cpp
    Private/Checker.cpp
    Private/CubeFormat.cpp
    Private/CubeString.cpp
    Private/Logger.cpp
)

set(PRECOMPILE_HEADER_FILES
    Public/Async.h
    Public/Blob.h
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
    Public/Quaternion.h
    Public/Types.h
    Public/Vector.h
)

if(PLATFORM_WINDOWS)
    set(PLATFORM_PUBLIC_FILES
        Public/Windows/WindowsDefines.h
        Public/Platform/Windows/WindowsDebug.h
        Public/Platform/Windows/WindowsDLib.h
        Public/Platform/Windows/WindowsFileSystem.h
        Public/Platform/Windows/WindowsPlatform.h
        Public/Platform/Windows/WindowsString.h
    )
    set(PLATFORM_PRIVATE_FILES
        Private/Platform/Windows/WindowsDebug.cpp
        Private/Platform/Windows/WindowsDLib.cpp
        Private/Platform/Windows/WindowsFileSystem.cpp
        Private/Platform/Windows/WindowsPlatform.cpp
        Private/Platform/Windows/WindowsString.cpp
    )
elseif(PLATFORM_MACOS)
    set(PLATFORM_PUBLIC_FILES
        Public/MacOS/MacOSDefines.h
        Public/Platform/MacOS/MacOSDebug.h
        Public/Platform/MacOS/MacOSDLib.h
        Public/Platform/MacOS/MacOSFileSystem.h
        Public/Platform/MacOS/MacOSLoggerSubprocess.h
        Public/Platform/MacOS/MacOSPlatform.h
        Public/Platform/MacOS/MacOSString.h
        Public/Platform/MacOS/MacOSUtility.h
    )
    set(PLATFORM_PRIVATE_FILES
        Private/Platform/MacOS/MacOSDebug.mm
        Private/Platform/MacOS/MacOSDLib.mm
        Private/Platform/MacOS/MacOSFileSystem.mm
        Private/Platform/MacOS/MacOSLoggerSubprocess.mm
        Private/Platform/MacOS/MacOSPlatform.mm
        Private/Platform/MacOS/MacOSString.mm
    )
    # Objective-C does not support precompiled headers
    set_source_files_properties(${PLATFORM_PRIVATE_FILES}
        PROPERTIES
            SKIP_PRECOMPILE_HEADERS ON
    )
    # Enable Automatic Reference Counting (ARC) for Objective-C++ files (.mm)
    set_source_files_properties(${PLATFORM_PRIVATE_FILES}
        PROPERTIES
            COMPILE_FLAGS "-fobjc-arc"
    )
endif()

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${PUBLIC_FILES} ${PRIVATE_FILES} ${PLATFORM_PUBLIC_FILES} ${PLATFORM_PRIVATE_FILES})

# Target settings
add_library(CE-Core SHARED
    ${GLOBAL_FILES}
    ${PUBLIC_FILES} ${PRIVATE_FILES}
    ${PLATFORM_PUBLIC_FILES} ${PLATFORM_PRIVATE_FILES}
)
target_include_directories(CE-Core
    PUBLIC
        Public
    PRIVATE
        Private
)
target_precompile_headers(CE-Core
    PUBLIC
        ${PRECOMPILE_HEADER_FILES}
)
target_compile_definitions(CE-Core
    PRIVATE
        CUBE_MODULE_CORE
)

# Third-party
target_link_libraries(CE-Core
    PUBLIC
        fmt::fmt
)
if(PLATFORM_WINDOWS)
    target_link_libraries(CE-Core
        PRIVATE
            Dbghelp.lib
    )
endif()
if(PLATFORM_MACOS)
    target_link_libraries(CE-Core
        PRIVATE
            "-framework AppKit"
    )
endif()
