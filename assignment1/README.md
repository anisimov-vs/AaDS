# AaDS Practical Work 1 - ASCII85 Encoder/Decoder

A C++ implementation of ASCII85 encoding and decoding utility, similar to the Unix base64 utility.

## Author

Анисимов Василий Сергеевич, группа 24.Б81-мм

## Contacts

st129629@student.spbu.ru

## Description

This project implements an ASCII85 encoder and decoder that can process input from STDIN and output to STDOUT. The utility supports both encoding and decoding operations, with options for different processing modes.

### Features

- ASCII85 encoding (converts binary data to ASCII85 format)
- ASCII85 decoding (converts ASCII85 format back to binary data)
- Two processing modes:
  - Stream mode: processes data gradually (default)
  - Buffer mode: reads entire input before processing
- Command-line options for different operations
- Comprehensive unit tests using GoogleTest
- Random data testing with Python base64 module integration

### Usage

```bash
# Encode (reads from STDIN, outputs ASCII85 to STDOUT)
ascii85
ascii85 -e

# Decode (reads ASCII85 from STDIN, outputs binary to STDOUT)
ascii85 -d

# Choose processing mode
ascii85        # Process data gradually (stream mode, default)
ascii85 -b     # Read entire input before processing (buffer mode)
```

### Build Instructions

#### Prerequisites

- C++17 compatible compiler
- CMake 3.10 or higher
- GoogleTest

#### Build

```bash
mkdir build
cd build
cmake ..
make
```

### Testing

The project includes:
- Unit tests for basic functionality
- Random data tests
- Integration tests with Python base64 module
- Error handling tests for invalid input

### Project Structure

- `src/`: Source code files
  - `ascii85.cpp`: Main implementation
  - `main.cpp`: Command-line interface
- `include/`: Header files
  - `ascii85.hpp`: ASCII85 class definition
- `tests/`: Test files
  - `ascii85_test.cpp`: Unit tests
  - `random_test.py`: Random data tests
- `CMakeLists.txt`: Build configuration

## License

This project is provided as-is for educational purposes. 