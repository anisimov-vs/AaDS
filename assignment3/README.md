# AaDS Practical Work 3 - Shannon-Fano Coder/Decoder

A C++ implementation of a Shannon-Fano coder and decoder utility for binary file compression.

## Author

Анисимов Василий Сергеевич, группа 24.Б81-мм

## Contacts

st129629@student.spbu.ru

## Description

This project implements a Shannon-Fano coder (compressor) and decoder (decompressor). The utility reads an input binary file, generates a frequency-based dictionary using the Shannon-Fano algorithm, and produces a compressed output file. The decoder uses the compressed file and the dictionary to reconstruct the original binary file.

### Features

- **Shannon-Fano Encoding**: Compresses binary data based on symbol frequencies.
- **Shannon-Fano Decoding**: Decompresses data using a provided dictionary.
- **Dictionary Management**:
    - Generates and saves a dictionary during encoding.
    - Reads and utilizes a dictionary during decoding.
- **Binary File Support**: Designed to work with any type of binary file.
- **Command-line Interface**: Options for specifying input, output, and dictionary files, as well as the operation mode (encode/decode).
- **Packet Testing**: A shell script (`test_script.sh`) is provided to compile the project and run a series of tests, verifying that the decoded output matches the original input for various file types.

### Usage

The utility requires specifying the mode (encode or decode), a dictionary file, and optionally input and output files (defaulting to STDIN/STDOUT).

**General command structure:**
`./shannon_fano_tool <mode> -t <dictionary_file> [-i <input_file>] [-o <output_file>]`

**Examples:**

```bash
# Encode a file 'original.dat' to 'compressed.sf', saving the dictionary to 'dict.dat'
./shannon_fano_tool -e -i original.dat -o compressed.sf -t dict.dat

# Decode 'compressed.sf' to 'decoded.dat' using 'dict.dat'
./shannon_fano_tool -d -i compressed.sf -o decoded.dat -t dict.dat

# Encode from STDIN to STDOUT
cat original.dat | ./shannon_fano_tool -e -t dict.dat > compressed.sf

# Decode from STDIN to STDOUT
cat compressed.sf | ./shannon_fano_tool -d -t dict.dat > decoded.dat