cmake_minimum_required(VERSION 3.10)
project(GaussianElimination VERSION 1.0)

# Specify C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(Eigen3 REQUIRED)

option(USE_BLAS "Use BLAS for Eigen operations" ON)

if(USE_BLAS)
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

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.0
)

FetchContent_MakeAvailable(googletest)

add_executable(gauss_test 
    tests/gaussian_elimination_test.cpp
    src/gaussian_elimination.cpp
)

target_link_libraries(gauss_test PRIVATE
    Eigen3::Eigen
    GTest::gtest
    GTest::gtest_main
)

if(USE_BLAS)
    target_link_libraries(gauss_test PRIVATE ${BLAS_LIBRARIES})
endif()

include(GoogleTest)
gtest_discover_tests(gauss_test)

add_test(NAME GaussianEliminationTests COMMAND gauss_test)
