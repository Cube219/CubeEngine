cmake_minimum_required(VERSION 3.19)

set(PUBLIC_FILES
    Public/DLib.h
    Public/FileSystem.h
    Public/Platform.h
    Public/PlatformDebug.h
    Public/PlatformHeader.h
)
set(PRIVATE_FILES
)

if(PLATFORM_WINDOWS)
    set(PLATFORM_PUBLIC_FILES
        Public/Windows/WindowsDebug.h
        Public/Windows/WindowsDLib.h
        Public/Windows/WindowsFileSystem.h
        Public/Windows/WindowsPlatform.h
        Public/Windows/WindowsString.h
    )
    set(PLATFORM_PRIVATE_FILES
        Private/Windows/WindowsDebug.cpp
        Private/Windows/WindowsDLib.cpp
        Private/Windows/WindowsFileSystem.cpp
        Private/Windows/WindowsPlatform.cpp
        Private/Windows/WindowsString.cpp
    )
elseif(PLATFORM_MACOS)
    set(PLATFORM_PUBLIC_FILES
        Public/MacOS/MacOSDebug.h
        Public/MacOS/MacOSDLib.h
        Public/MacOS/MacOSFileSystem.h
        Public/MacOS/MacOSPlatform.h
        Public/MacOS/MacOSString.h
    )
    set(PLATFORM_PRIVATE_FILES
        Private/MacOS/MacOSDebug.cpp
        Private/MacOS/MacOSDLib.cpp
        Private/MacOS/MacOSFileSystem.cpp
        Private/MacOS/MacOSPlatform.cpp
        Private/MacOS/MacOSString.cpp
    )
endif()

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${PUBLIC_FILES} ${PRIVATE_FILES} ${PLATFORM_PUBLIC_FILES} ${PLATFORM_PRIVATE_FILES})

# Target settings
add_library(CE-Platform SHARED
    ${PUBLIC_FILES} ${PRIVATE_FILES}
    ${PLATFORM_PUBLIC_FILES} ${PLATFORM_PRIVATE_FILES}
)
target_include_directories(CE-Platform
    PUBLIC
        Public
    PRIVATE
        Private
)
target_link_libraries(CE-Platform
    PRIVATE
        CE-Base
        CE-Logger
)
if(PLATFORM_WINDOWS)
    target_link_libraries(CE-Platform
        PRIVATE
            Dbghelp.lib
    )
endif()
target_compile_definitions(CE-Platform
    PRIVATE
        CUBE_MODULE_PLATFORM
)
