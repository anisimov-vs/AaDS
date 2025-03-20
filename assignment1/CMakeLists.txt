cmake_minimum_required(VERSION 3.10)
project(ascii85 VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Option to build tests
option(BUILD_TESTS "Build the tests" ON)

# Add source files
set(SOURCES
    src/main.cpp
    src/ascii85.cpp
)

# Add header files
set(HEADERS
    include/ascii85.hpp
)

# Create main executable
add_executable(ascii85 ${SOURCES} ${HEADERS})

# Add include directories
target_include_directories(ascii85 PRIVATE include)

# Build tests if enabled
if(BUILD_TESTS)
    message(STATUS "Building tests with Google Test")
    
    # Set up Google Test
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
    )
    FetchContent_MakeAvailable(googletest)
    
    # Add test executable
    add_executable(ascii85_test
        tests/ascii85_test.cpp
        src/ascii85.cpp
    )
    
    # Add include directories for test
    target_include_directories(ascii85_test PRIVATE include)
    
    # Link test executable with Google Test
    target_link_libraries(ascii85_test PRIVATE gtest gtest_main)
    
    # Enable testing
    enable_testing()
    include(GoogleTest)
    gtest_discover_tests(ascii85_test)
    
    # Add test to project
    add_test(NAME ascii85_test COMMAND ascii85_test)
else()
    message(STATUS "Tests disabled. Use -DBUILD_TESTS=ON to enable.")
endif() 