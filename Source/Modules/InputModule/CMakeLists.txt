cmake_minimum_required (VERSION 3.6)

set(INCLUDE
    InputModule.h
    InputModuleFwd.h
    InputModuleHeader.h
    KeyboardMouseInput.h
    XboxControllerInput.h
)
set(SOURCE
    InputModule.cpp
    KeyboardMouseInput.cpp
    XboxControllerInput.cpp
)

# Target
add_library(CE-InputModule SHARED ${INCLUDE} ${SOURCE})

# Defines
target_compile_definitions(CE-InputModule PRIVATE INPUTMODULE_EXPORTS)

# Internal libs
target_link_libraries(CE-InputModule PUBLIC CE-Utility CE-Platform CE-Core)

# Platform-dependent
if(PLATFORM_WIN32)
    # Xinput lib
    target_link_libraries(CE-InputModule PRIVATE Xinput.lib)
endif()
