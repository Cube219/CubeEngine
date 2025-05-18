cmake_minimum_required(VERSION 3.19)

# Target settings
if(PLATFORM_WINDOWS)
    add_executable(CE-Main WIN32 main.cpp)
endif()
if(PLATFORM_MACOS)
    set(MACOSX_BUNDLE TRUE)
    add_executable(CE-Main MACOSX_BUNDLE main.cpp)
    
    set_target_properties(CE-Main PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${RESOURCES_DIR}/MacOS/info.plist")
endif()
target_link_libraries(CE-Main
    PRIVATE
        CE-Base
        CE-Platform
        CE-GAPI
        CE-Core
)

# Add GAPI dependencies to avoid omitting GAPI compilation
if(G_ENABLED_GAPI_MODULES)
    add_dependencies(CE-Main ${G_ENABLED_GAPI_MODULES})
endif()

# Other settings
set_target_properties(CE-Main PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:CE-Main>")
