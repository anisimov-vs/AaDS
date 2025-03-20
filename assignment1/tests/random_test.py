#!/usr/bin/env python3

import subprocess
import random
import base64
import sys
import os

def generate_random_data(size):
    return bytes(random.randint(0, 255) for _ in range(size))

def test_encoding(data):
    # Encode using our C++ program
    process = subprocess.Popen(['./ascii85'], stdin=subprocess.PIPE, stdout=subprocess.PIPE)
    encoded, _ = process.communicate(input=data)
    
    # Encode using Python's base64
    python_encoded = base64.a85encode(data)
    
    # Compare results
    return encoded.strip() == python_encoded.strip()

def test_decoding(data):
    # First encode using Python's base64
    encoded = base64.a85encode(data)
    
    # Decode using our C++ program
    process = subprocess.Popen(['./ascii85', '-d'], stdin=subprocess.PIPE, stdout=subprocess.PIPE)
    decoded, _ = process.communicate(input=encoded)

    #print(decoded, data)
    
    # Compare results
    return decoded == data

def test_invalid_input():
    # Test for invalid character in ASCII85 input (out of range)
    # The ^ character (ASCII 94) is outside the valid ASCII85 range ('!' to 'u')
    invalid_chars = b"^invalid"
    process = subprocess.Popen(['./ascii85', '-d'], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    _, _ = process.communicate(input=invalid_chars)
    if process.returncode == 0:
        print("✗ Test failed: Invalid characters accepted")
        return False
    else:
        print("✓ Test passed: Invalid characters rejected")
    
    return True

def main():
    # Test with different data sizes
    sizes = [1, 4, 8, 16, 32, 64, 128, 256, 512, 1024]
    
    print("Running random data tests...")
    
    # Test encoding
    print("\nTesting encoding:")
    for size in sizes:
        data = generate_random_data(size)
        if test_encoding(data):
            print(f"✓ Encoding test passed for size {size}")
        else:
            print(f"✗ Encoding test failed for size {size}")
            return 1
    
    # Test decoding
    print("\nTesting decoding:")
    for size in sizes:
        data = generate_random_data(size)
        if test_decoding(data):
            print(f"✓ Decoding test passed for size {size}")
        else:
            print(f"✗ Decoding test failed for size {size}")
            return 1
    
    # Test invalid input
    print("\nTesting invalid input:")
    if test_invalid_input():
        print("✓ Invalid input test passed")
    else:
        print("✗ Invalid input test failed")
        return 1
    
    print("\nAll tests passed!")
    return 0

if __name__ == "__main__":
    sys.exit(main()) 