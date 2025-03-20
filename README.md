# Algorithms and Data Structures (AaDS)

This repository contains laboratory work assignments for the Algorithms and Data Structures course.

## Author

Анисимов Василий Сергеевич, группа 24.Б81-мм

## Contacts

st129629@student.spbu.ru

## Assignments

- [Assignment 1](./assignment1): ASCII85 Encoder/Decoder - A C++ implementation of ASCII85 encoding and decoding utility

## Building and Testing

Each assignment can be built and tested individually:

```bash
# Navigate to assignment directory
cd assignmentX

# Build
mkdir -p build
cd build
cmake ..
make

# Run tests
cd ..
./test_script.sh
```

Alternatively, GitHub Actions automatically builds and tests all assignments in the repository.

## Project Structure

Each assignment follows a similar structure:

- `src/`: Source code files
- `include/`: Header files
- `tests/`: Test files
- `CMakeLists.txt`: Build configuration
- `README.md`: Assignment description
- `test_script.sh`: Testing script

## License

This project is provided as-is for educational purposes. 