cmake_minimum_required (VERSION 3.6)

set(INCLUDE
    Engine.h
    EngineFwd.h
    EngineHeader.h
)
set(SOURCE
    Engine.cpp
)

# Target
add_library(CE-Engine SHARED ${INCLUDE} ${SOURCE})

# Defines
target_compile_definitions(CE-Engine PRIVATE ENGINE_EXPORTS)

# Internal libs
target_link_libraries(CE-Engine PUBLIC CE-Utility CE-Platform CE-Core)
