#!/bin/bash

# Exit on any error
set -e

# Check if build exists, and build if necessary
if [ ! -d "build" ]; then
    echo "Build directory not found. Running build script first..."
    if ! ./build.sh; then
        echo "Build script failed. Exiting."
        exit 1
    fi
fi

# Check for solver executable (critical)
if [ ! -f "build/gauss_solver" ]; then
    echo "Solver executable (build/gauss_solver) not found. Make sure the build was successful. Exiting."
    exit 1
fi

# Check for test executable (critical)
if [ ! -f "build/gauss_test" ]; then
    echo "Unit test executable (build/gauss_test) not found. Make sure the build was successful. Exiting."
    exit 1
fi

# Run the unit tests
echo "Running unit tests..."
./build/gauss_test
echo "Unit tests completed."

# Run the solver on the example input
echo -e "\nRunning example with known solution:"
./build/gauss_solver --input data/input_example1.csv --output solution.csv

echo -e "\nComparing solution with expected result:"

if [ -f "solution.csv" ] && [ -f "data/expected_solution1.csv" ]; then
    echo "Expected solution:"
    cat data/expected_solution1.csv
    
    echo -e "\nCalculated solution:"
    cat solution.csv
    
    echo -e "\nDifference:"
    if diff -q solution.csv data/expected_solution1.csv >/dev/null; then
        echo "PASSED: Solution matches expected values!"
    else
        echo "FAILED: Solution differs from expected values!"
        echo "Showing differences:"
        diff solution.csv data/expected_solution1.csv
        exit 1
    fi
else
    echo "Solution file (solution.csv) or expected solution file (data/expected_solution1.csv) not found."
    exit 1
fi

# Generate and solve a larger random system
echo -e "\nGenerating and solving a 20x20 random system:"
./build/gauss_solver --generate 20 --seed 42 --matrix-out random_system.csv --output random_solution.csv

echo "Completed random system test."

echo -e "\nAll tests completed!"
