#!/bin/bash

BUILD_DIR="build"
EXECUTABLE_NAME="shannon_fano_tool"
EXECUTABLE_PATH="${BUILD_DIR}/src/${EXECUTABLE_NAME}"

print_result() {
    if [ "$1" -eq 0 ]; then
        echo "✅ PASSED: $2"
    else
        echo "❌ FAILED: $2"
        exit 1
    fi
}

cleanup() {
    rm -f test_original.txt test_compressed.sf test_decoded.txt test_dict.sf \
          test_original.bin test_compressed_bin.sf test_decoded_bin.bin test_dict_bin.sf \
          empty_original.txt empty_compressed.sf empty_decoded.txt empty_dict.sf \
          single_char_original.txt single_char_compressed.sf single_char_decoded.txt single_char_dict.sf \
          all_different_original.bin all_different_compressed.sf all_different_decoded.bin all_different_dict.sf
}

set -e
trap cleanup EXIT

if [ ! -d "${BUILD_DIR}" ]; then
    mkdir "${BUILD_DIR}"
fi
cd "${BUILD_DIR}"
cmake ..
cmake --build . --config Release
cd ..

if [ ! -f "${EXECUTABLE_PATH}" ]; then
    EXECUTABLE_PATH_ALT="${BUILD_DIR}/${EXECUTABLE_NAME}"
    if [ ! -f "${EXECUTABLE_PATH_ALT}" ]; then
        echo "Error: Executable not found."
        exit 1
    else
        EXECUTABLE_PATH="${EXECUTABLE_PATH_ALT}"
    fi
fi

echo "Testing Shannon-Fano Tool..."

echo "Hello Shannon-Fano! Test." > test_original.txt
echo "AAAAABBBCCCDDEEFF" >> test_original.txt
"${EXECUTABLE_PATH}" -e -i test_original.txt -o test_compressed.sf -t test_dict.sf
"${EXECUTABLE_PATH}" -d -i test_compressed.sf -o test_decoded.txt -t test_dict.sf
diff test_original.txt test_decoded.txt
print_result $? "Simple text file"

dd if=/dev/urandom of=test_original.bin bs=1024 count=1 status=none
"${EXECUTABLE_PATH}" -e -i test_original.bin -o test_compressed_bin.sf -t test_dict_bin.sf
"${EXECUTABLE_PATH}" -d -i test_compressed_bin.sf -o test_decoded_bin.bin -t test_dict_bin.sf
cmp -s test_original.bin test_decoded_bin.bin
print_result $? "Random binary file"

touch empty_original.txt
"${EXECUTABLE_PATH}" -e -i empty_original.txt -o empty_compressed.sf -t empty_dict.sf
"${EXECUTABLE_PATH}" -d -i empty_compressed.sf -o empty_decoded.txt -t empty_dict.sf
cmp -s empty_original.txt empty_decoded.txt
print_result $? "Empty file"

printf 'AAAAA' > single_char_original.txt
"${EXECUTABLE_PATH}" -e -i single_char_original.txt -o single_char_compressed.sf -t single_char_dict.sf
"${EXECUTABLE_PATH}" -d -i single_char_compressed.sf -o single_char_decoded.txt -t single_char_dict.sf
diff single_char_original.txt single_char_decoded.txt
print_result $? "Single character file"

head -c 256 /dev/urandom > all_different_original.bin
"${EXECUTABLE_PATH}" -e -i all_different_original.bin -o all_different_compressed.sf -t all_different_dict.sf
"${EXECUTABLE_PATH}" -d -i all_different_compressed.sf -o all_different_decoded.bin -t all_different_dict.sf
cmp -s all_different_original.bin all_different_decoded.bin
print_result $? "File with many different byte values (random 256 bytes)"

echo "All tests completed successfully."