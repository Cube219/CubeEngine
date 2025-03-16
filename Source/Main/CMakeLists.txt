cmake_minimum_required(VERSION 3.19)

# Target settings
add_executable(CE-Main WIN32 main.cpp)
target_link_libraries(CE-Main
    PRIVATE
        CE-Base
        CE-GAPI
        CE-Core
)

# Other settings
set_target_properties(CE-Main PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:CE-Main>")
