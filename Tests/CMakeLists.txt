cmake_minimum_required(VERSION 3.19)

set(TEST_FILES
    VectorTest.cpp
    MatrixTest.cpp
    MatrixUtilityTest.cpp
)

add_executable(CE-Tests ${TEST_FILES})

target_link_libraries(CE-Tests
    PRIVATE
        CE-Base
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(CE-Tests)
