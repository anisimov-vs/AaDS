cmake_minimum_required(VERSION 3.10)
project(GaussianElimination VERSION 1.0)

# Specify C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find Eigen3 package
find_package(Eigen3 REQUIRED)

# Options
option(USE_BLAS "Use BLAS for Eigen operations" ON)

if(USE_BLAS)
    # Find OpenBLAS
    find_package(BLAS REQUIRED)
    add_definitions(-DEIGEN_USE_BLAS -DEIGEN_USE_LAPACKE)
endif()

# Add the include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${EIGEN3_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/externals/lazycsv/include
)

# Define the executable
add_executable(gauss_solver 
    src/main.cpp
    src/gaussian_elimination.cpp
)

# Link libraries
target_link_libraries(gauss_solver 
    Eigen3::Eigen
)

if(USE_BLAS)
    target_link_libraries(gauss_solver ${BLAS_LIBRARIES})
endif()

# Tests
enable_testing()

# Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1 # Or another appropriate tag/commit
)
# For newer CMake versions (3.11+), GTest_SOURCE_DIR and GTest_BINARY_DIR are not needed
# For older versions, you might need:
# Set(GTest_SOURCE_DIR ${CMAKE_BINARY_DIR}/googletest-src)
# Set(GTest_BINARY_DIR ${CMAKE_BINARY_DIR}/googletest-build)
# FetchContent_GetProperties(googletest)
# if(NOT googletest_POPULATED)
#   FetchContent_Populate(googletest)
#   add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})
# endif()
FetchContent_MakeAvailable(googletest) # This should define GTest::gtest and GTest::gtest_main

add_executable(gauss_test 
    tests/gaussian_elimination_test.cpp
    src/gaussian_elimination.cpp
)

target_link_libraries(gauss_test PRIVATE
    Eigen3::Eigen
    GTest::gtest
    GTest::gtest_main # Or gtest_main if GTest::gtest_main is not found
)

if(USE_BLAS)
    target_link_libraries(gauss_test PRIVATE ${BLAS_LIBRARIES})
endif()

include(GoogleTest)
gtest_discover_tests(gauss_test)

# The add_test call might be redundant if gtest_discover_tests is used,
# but it depends on the CMake version and GTest integration.
# For clarity or older setups, it can be kept.
add_test(NAME GaussianEliminationTests COMMAND gauss_test)
