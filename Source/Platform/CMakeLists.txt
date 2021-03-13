cmake_minimum_required (VERSION 3.6)

set(INCLUDE
    DLib.h
    FileSystem.h
    Platform.h
    PlatformDebug.h
    PlatformHeader.h
    PlatformString.h
)
set(SOURCE
)

if(PLATFORM_WIN32)
    set(PLATFORM_INCLUDE
        Win32/Win32Debug.h
        Win32/Win32DLib.h
        Win32/Win32FileSystem.h
        Win32/Win32Platform.h
    )
    set(PLATFORM_SOURCE
        Win32/Win32Debug.cpp
        Win32/Win32DLib.cpp
        Win32/Win32FileSystem.cpp
        Win32/Win32Platform.cpp
        Win32/Win32String.cpp
    )
endif()

# Target
add_library(CE-Platform SHARED ${INCLUDE} ${SOURCE} ${PLATFORM_INCLUDE} ${PLATFORM_SOURCE})

# Defines
target_compile_definitions(CE-Platform PRIVATE PLATFORM_EXPORTS)

# Internal libs
target_link_libraries(CE-Platform PUBLIC CE-Utility)
