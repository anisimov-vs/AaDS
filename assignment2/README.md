# AaDS Practical Work 1 - Gaussian Elimination Solver

A C++ program to solve systems of linear equations using Gaussian elimination with partial pivoting. It can read systems from CSV files or generate random ones.

## Author

Анисимов Василий Сергеевич, группа 24.Б81-мм

## Contacts

st129629@student.spbu.ru

## Features

- Solves linear systems from CSV files using Gaussian elimination with partial pivoting.
- Generates random linear systems with specified dimensions and seed.
- Writes solution vectors to CSV files.
- Command-line interface for specifying input, output, and generation parameters.
- Includes a comprehensive test suite using Google Test.

## Requirements

- C++17 compatible compiler
- CMake (version 3.10 or later)
- Eigen3 library
- OpenBLAS
- Google Test framework

## Building the Project

1.  **Using the script (recommended):**
    ```bash
    ./build.sh
    ```
    This will create a `build` directory, run CMake, and compile the project.

2.  **Manual build:**
    ```bash
    mkdir -p build
    cd build
    cmake ..
    make
    ```

## Usage

The `gauss_solver` executable is created in the `build` directory.

### Command-Line Options

```
Usage: gauss_solver [options]
Options:
  --input <file>      Input CSV file for the augmented matrix [A|b].
                      Each row is an equation, last column is the constant.
                      Example: 2,1,-1,8 (for 2x+y-z=8)
  --output <file>     Output CSV file for the solution vector (one element per line).
  --generate <N>      Generate a random N-equation system.
  --seed <S>          Seed for random number generator.
  --min <val>         Min value for random coefficients (default: -10.0).
  --max <val>         Max value for random coefficients (default: 10.0).
  --matrix-out <file> Save generated matrix to this file (with --generate).
  --help              Display this help message.
```

### Examples

1.  Solve a system from `data/input_example1.csv` and save to `solution.csv`:
    ```bash
    ./build/gauss_solver --input data/input_example1.csv --output solution.csv
    ```

2.  Generate a random 10x10 system, save it, and solve it:
    ```bash
    ./build/gauss_solver --generate 10 --seed 42 --matrix-out random_system.csv --output random_solution.csv
    ```

## Running Tests

The project includes unit tests and integration tests. To run all tests:

```bash
./test_script.sh
```

## License

This project is provided as is for educational purposes.
