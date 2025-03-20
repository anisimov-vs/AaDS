#include "ascii85.hpp"
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <array>
#include <cstring>
#include <unistd.h> // For isatty() function

namespace ascii85 {

// Adobe ASCII85 uses characters starting with '!' (33) and ending with 'u' (117)
const char* ASCII85::ENCODING_TABLE = "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstu";
const uint32_t ASCII85::powers[5] = {85*85*85*85, 85*85*85, 85*85, 85, 1};

// Initialize decoding table
uint8_t ASCII85::DECODING_TABLE[256] = {0};
struct DecodingTableInitializer {
    DecodingTableInitializer() {
        // Initialize all values to invalid (255)
        std::fill_n(ASCII85::DECODING_TABLE, 256, 255);
        
        // Set valid character mappings
        for (int i = 0; i < 85; i++) {
            ASCII85::DECODING_TABLE[static_cast<uint8_t>(ASCII85::ENCODING_TABLE[i])] = i;
        }
    }
} decodingTableInitializer;

std::string ASCII85::encode(const std::string& input) {
    if (input.empty()) {
        return "";
    }
    
    std::stringstream output;
    const unsigned char* data = reinterpret_cast<const unsigned char*>(input.data());
    size_t length = input.length();
    
    // Adobe ASCII85 encoding: Process input in chunks of 4 bytes
    for (size_t i = 0; i < length; i += 4) {
        // Determine how many bytes we have in this chunk (1-4)
        size_t bytesInChunk = std::min(length - i, size_t(4));
        
        // Calculate the 32-bit value for these bytes (big-endian)
        uint32_t value = 0;
        for (size_t j = 0; j < bytesInChunk; j++) {
            value |= static_cast<uint32_t>(data[i + j]) << (8 * (3 - j));
        }
        
        // Special case: all zeros
        if (value == 0 && bytesInChunk == 4) {
            output << 'z';
            continue;
        }
        
        // Convert value to base-85 (5 ASCII85 characters)
        char encoded[5];
        for (int j = 0; j < 5; j++) {
            encoded[j] = '!' + (value / powers[j] % 85);
        }
        
        // Output only as many characters as needed (n+1 for n input bytes)
        output.write(encoded, bytesInChunk + 1);
    }
    
    return output.str();
}

std::string ASCII85::decode(const std::string& input) {
    if (input.empty()) {
        return "";
    }
    
    // Check for non-ASCII characters
    for (size_t i = 0; i < input.length(); ++i) {
        unsigned char c = static_cast<unsigned char>(input[i]);
        if (c > 127) {
            throw std::runtime_error("Invalid ASCII85 input: character out of range");
        }
    }
    
    // Verify the entire input is valid - check for incomplete groups
    // Count the number of valid characters (excluding whitespace and 'z')
    size_t validCharCount = 0;
    bool hasZ = false;
    
    for (size_t i = 0; i < input.length(); ++i) {
        char c = input[i];
        if (c > ' ' && c != 'z') {
            validCharCount++;
        } else if (c == 'z') {
            hasZ = true;
        }
    }
    
    // If there are still non-whitespace characters after removing all valid 
    // complete groups (multiples of 5) and 'z' characters, validate that
    // the remaining count makes sense for a final partial group (must be 2-4 chars)
    if (validCharCount % 5 != 0) {
        size_t remainder = validCharCount % 5;
        // Valid incomplete groups must be 2-4 characters
        if (remainder == 1) {
            throw std::runtime_error("Invalid ASCII85 input: incomplete group");
        }
    }
    
    std::stringstream output;
    const char* data = input.data();
    size_t length = input.length();
    
    // Process input in chunks
    size_t i = 0;
    while (i < length) {
        // Handle special 'z' case (four zero bytes)
        if (data[i] == 'z') {
            // Check if 'z' is used incorrectly (within a group)
            if (i > 0 && i < length - 1) {
                // Check if the previous or next character is part of a group (not whitespace)
                bool prev_is_group = (i > 0 && data[i-1] > ' ' && data[i-1] != 'z');
                bool next_is_group = (i < length - 1 && data[i+1] > ' ' && data[i+1] != 'z');
                if (prev_is_group || next_is_group) {
                    throw std::runtime_error("Invalid ASCII85 input: 'z' character in wrong context");
                }
            }
            
            output.write("\0\0\0\0", 4);
            i++;
            continue;
        }
        
        // Read up to 5 characters to form a group
        uint32_t value = 0;
        int charCount = 0;
        size_t groupStart = i;
        
        for (int j = 0; j < 5 && i < length; j++, i++) {
            char c = data[i];
            
            // Skip whitespace
            if (c <= ' ') {
                j--; // Don't count whitespace
                continue;
            }
            
            // Ensure character is in valid range
            if (c < '!' || c > 'u') {
                throw std::runtime_error("Invalid ASCII85 input: character out of range");
            }
            
            // Convert from ASCII85 to value
            value = value * 85 + (c - '!');
            charCount++;
            
            // Check for overflow during calculation
            if (value > 0xFFFFFFFF && j < 4) {
                throw std::runtime_error("Invalid ASCII85 input: value overflow");
            }
        }
        
        // Need at least 1 character to decode
        if (charCount == 0) {
            continue;
        }
        
        // Handle incomplete groups (1-4 chars) with validation
        if (charCount < 5) {
            // A single character group is always invalid
            if (charCount == 1) {
                throw std::runtime_error("Invalid ASCII85 input: incomplete group");
            }
            
            // If we reached the end of input, this is expected
            if (groupStart + charCount >= length) {
                // Valid end of stream with partial group
                // Pad with 'u' characters if needed
                for (int j = charCount; j < 5; j++) {
                    value = value * 85 + 84; // 'u' - '!' = 84
                    // Check for overflow after padding
                    if (value > 0xFFFFFFFF) {
                        throw std::runtime_error("Invalid ASCII85 input: value overflow");
                    }
                }
            } else {
                // If we didn't reach the end of input but have an incomplete group,
                // this is invalid unless it's due to whitespace
                bool hasOnlyWhitespace = true;
                for (size_t j = groupStart + charCount; j < length; j++) {
                    if (data[j] > ' ') {
                        hasOnlyWhitespace = false;
                        break;
                    }
                }
                
                if (!hasOnlyWhitespace) {
                    throw std::runtime_error("Invalid ASCII85 input: incomplete group");
                }
                
                // Pad with 'u' characters if needed
                for (int j = charCount; j < 5; j++) {
                    value = value * 85 + 84; // 'u' - '!' = 84
                    // Check for overflow after padding
                    if (value > 0xFFFFFFFF) {
                        throw std::runtime_error("Invalid ASCII85 input: value overflow");
                    }
                }
            }
        }
        
        // Final check for overflow - max 32-bit value is 0xFFFFFFFF
        if (value > 0xFFFFFFFF) {
            throw std::runtime_error("Invalid ASCII85 input: value overflow");
        }
        
        // Determine how many bytes to output based on input char count
        int bytesToOutput = charCount == 5 ? 4 : charCount - 1;
        
        // Convert to bytes and output
        for (int j = 0; j < bytesToOutput; j++) {
            unsigned char byte = (value >> (8 * (3 - j))) & 0xFF;
            output.put(byte);
        }
    }
    
    return output.str();
}

void ASCII85::processStream(std::istream& input, std::ostream& output, bool decode) {
    if (decode) {
        // Decoding implementation - process data character by character
        char c;
        std::string group;
        
        while (input.get(c)) {
            // Handle 'z' special case immediately
            if (c == 'z') {
                output.write("\0\0\0\0", 4);
                continue;
            }
            
            // Skip whitespace
            if (c <= ' ') {
                // If we have a complete group, process it
                if (group.length() > 0) {
                    std::string decoded = ASCII85::decode(group);
                    output.write(decoded.data(), decoded.length());
                    group.clear();
                }
                continue;
            }
            
            // Validate character
            if (c < '!' || c > 'u') {
                throw std::runtime_error("Invalid ASCII85 input: character out of range");
            }
            
            // Add character to group
            group += c;
            
            // Process complete group (5 characters)
            if (group.length() == 5) {
                std::string decoded = ASCII85::decode(group);
                output.write(decoded.data(), decoded.length());
                group.clear();
            }
        }
        
        // Process any remaining characters
        if (!group.empty()) {
            std::string decoded = ASCII85::decode(group);
            output.write(decoded.data(), decoded.length());
        }
    } else {
        // Encoding implementation - handle both interactive (line-based) and piped input
        const size_t BUFFER_SIZE = 4096;
        
        // First check if stdin is a terminal (interactive) or a pipe
        bool isInteractive = isatty(fileno(stdin));
        
        if (isInteractive) {
            // Interactive mode - process line by line
            std::string line;
            while (std::getline(input, line)) {
                if (!line.empty()) {
                    // Encode and output immediately
                    std::string encoded = ASCII85::encode(line);
                    output << encoded << std::endl; // Add newline for better usability
                    output.flush(); // Make sure output is displayed immediately
                }
            }
        } else {
            // Pipe mode - process in chunks for efficiency
            std::vector<char> buffer(BUFFER_SIZE);
            
            while (input) {
                input.read(buffer.data(), BUFFER_SIZE);
                std::streamsize bytesRead = input.gcount();
                
                if (bytesRead > 0) {
                    std::string data(buffer.data(), bytesRead);
                    std::string encoded = ASCII85::encode(data);
                    output << encoded;
                }
            }
        }
    }
}

void ASCII85::processBuffer(const std::string& data, std::ostream& output, bool decode) {
    if (decode) {
        std::string decoded = ASCII85::decode(data);
        output.write(decoded.data(), decoded.length());
    } else {
        std::string encoded = ASCII85::encode(data);
        output << encoded;
    }
}

void ASCII85::process(std::istream& input, std::ostream& output, Mode mode, bool decode) {
    if (mode == Mode::STREAM) {
        processStream(input, output, decode);
    } else {
        std::stringstream buffer;
        buffer << input.rdbuf();
        std::string data = buffer.str();
        processBuffer(data, output, decode);
    }
}

} // namespace ascii85