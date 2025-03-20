#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <cstdint>

namespace ascii85 {

class ASCII85 {
public:
    enum class Mode {
        STREAM,
        BUFFER
    };

    // Encode binary data to ASCII85
    static std::string encode(const std::string& input);
    
    // Decode ASCII85 to binary data
    static std::string decode(const std::string& input);
    
    // Process input stream in stream mode
    static void processStream(std::istream& input, std::ostream& output, bool decode = false);
    
    // Process input stream in buffer mode
    static void processBuffer(const std::string& data, std::ostream& output, bool decode = false);
    
    // Process input stream with specified mode
    static void process(std::istream& input, std::ostream& output, Mode mode, bool decode = false);

    // Encoding/decoding tables
    static const char* ENCODING_TABLE;
    static uint8_t DECODING_TABLE[256];
    static const uint32_t powers[5];

private:
    // Helper functions
    static uint32_t decodeGroup(const char* group);
    static std::string encodeGroup(uint32_t value);
    static bool isValidASCII85(const std::string& data);
};

} // namespace ascii85 