#!/bin/bash

# Change to the build directory
cd build || { echo "Error: build directory not found"; exit 1; }

# Unit tests
./ascii85_test

# Test with a simple string
echo "Testing ASCII85 encoding and decoding with a simple string"
TESTSTR="Hello, World!"
ENCODED=$(echo -n "$TESTSTR" | ./ascii85)
DECODED=$(echo -n "$ENCODED" | ./ascii85 -d)

echo "Original: $TESTSTR"
echo "Encoded: $ENCODED"
echo "Decoded: $DECODED"

if [ "$DECODED" = "$TESTSTR" ]; then
    echo "✅ Test passed: Encoded and decoded strings match"
else
    echo "❌ Test failed: Encoded and decoded strings don't match"
    exit 1
fi

# Test with a binary file
echo -e "\nTesting ASCII85 encoding and decoding with binary data"
dd if=/dev/urandom of=random_binary bs=1024 count=1 2>/dev/null
./ascii85 < random_binary > encoded_binary
./ascii85 -d < encoded_binary > decoded_binary

if cmp -s random_binary decoded_binary; then
    echo "✅ Test passed: Binary data properly encoded and decoded"
else
    echo "❌ Test failed: Binary data encoding/decoding failed"
    exit 1
fi

# Test the stream and buffer modes
echo -e "\nTesting stream and buffer modes"
echo -n "$TESTSTR" | ./ascii85 > stream_encoded
echo -n "$TESTSTR" | ./ascii85 -b > buffer_encoded

if cmp -s stream_encoded buffer_encoded; then
    echo "✅ Test passed: Stream and buffer modes produce identical output"
else
    echo "❌ Test failed: Stream and buffer modes produce different output"
    exit 1
fi

# Test invalid input
echo -e "\nTesting invalid input handling"
echo -n "Invalid^ASCII85^Input" | ./ascii85 -d >/dev/null 2>&1
if [ $? -ne 0 ]; then
    echo "✅ Test passed: Decoder correctly fails on invalid input"
else
    echo "❌ Test failed: Decoder doesn't fail on invalid input"
    exit 1
fi

python ../tests/random_test.py

exit 0 