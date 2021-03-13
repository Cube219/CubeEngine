cmake_minimum_required (VERSION 3.6)

set(INCLUDE
)
set(SOURCE
    main.cpp
)

# Target
add_executable(main WIN32 ${INCLUDE} ${SOURCE})

# Internal libs
target_link_libraries(main PUBLIC CE-Engine)
