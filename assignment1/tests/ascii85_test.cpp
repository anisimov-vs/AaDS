#include <gtest/gtest.h>
#include "ascii85.hpp"
#include <sstream>

using namespace ascii85;

TEST(ASCII85Test, EncodeEmpty) {
    std::string input = "";
    std::string result = ASCII85::encode(input);
    EXPECT_EQ(result, "");
}

TEST(ASCII85Test, DecodeEmpty) {
    std::string input = "";
    std::string result = ASCII85::decode(input);
    EXPECT_EQ(result.size(), 0);
}

TEST(ASCII85Test, EncodeSingleByte) {
    std::string input = "B";  // ASCII for 0x42
    std::string result = ASCII85::encode(input);
    EXPECT_EQ(result, "63");
}

TEST(ASCII85Test, DecodeSingleByte) {
    std::string input = "63";
    std::string result = ASCII85::decode(input);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], 'B');  // ASCII for 0x42
}

TEST(ASCII85Test, EncodeFourBytes) {
    std::string input = "BCDE";  // ASCII for 0x42, 0x43, 0x44, 0x45
    std::string result = ASCII85::encode(input);
    // In ASCII85, 4 bytes are encoded as 5 characters
    // For bytes 0x42,0x43,0x44,0x45 (BCDE), the encoding is "6:4.0"
    EXPECT_EQ(result, "6:4.0");
}

TEST(ASCII85Test, DecodeFourBytes) {
    std::string input = "6:4.0";
    std::string result = ASCII85::decode(input);
    EXPECT_EQ(result.size(), 4);
    EXPECT_EQ(result[0], 'B');  // ASCII for 0x42
    EXPECT_EQ(result[1], 'C');  // ASCII for 0x43
    EXPECT_EQ(result[2], 'D');  // ASCII for 0x44
    EXPECT_EQ(result[3], 'E');  // ASCII for 0x45
}

TEST(ASCII85Test, EncodeAllZeros) {
    std::string input(4, '\0');  // 4 zero bytes
    std::string result = ASCII85::encode(input);
    // In ASCII85, 4 zero bytes are encoded as a single 'z'
    EXPECT_EQ(result, "z");
}

TEST(ASCII85Test, DecodeAllZeros) {
    std::string input = "z";
    std::string result = ASCII85::decode(input);
    EXPECT_EQ(result.size(), 4);
    EXPECT_EQ(result[0], '\0');
    EXPECT_EQ(result[1], '\0');
    EXPECT_EQ(result[2], '\0');
    EXPECT_EQ(result[3], '\0');
}

TEST(ASCII85Test, InvalidInput) {
    // Test multiple invalid input scenarios
    
    // 1. Characters outside the valid ASCII85 range
    std::string invalidChars = "Invalid^ASCII85{Data";
    EXPECT_THROW(ASCII85::decode(invalidChars), std::runtime_error);
    
    // 2. z character in wrong context
    std::string invalidZ = "ABCz";  // 'z' should be by itself, not in a group
    EXPECT_THROW(ASCII85::decode(invalidZ), std::runtime_error);
    
    // 3. Mixed valid and invalid characters
    std::string mixedInput = "6:4.0\x80"; // Valid followed by invalid
    EXPECT_THROW(ASCII85::decode(mixedInput), std::runtime_error);
    
    // 4. Overly large values that would overflow
    // ASCII85 max value is 0xFFFFFFFF, so a group that decodes to larger would be invalid
    std::string overflowValue = "sss:z"; // This should decode to a value > 0xFFFFFFFF
    EXPECT_THROW(ASCII85::decode(overflowValue), std::runtime_error);
}

TEST(ASCII85Test, StreamMode) {
    std::string input = "Hello, World!";
    std::stringstream in(input);
    std::stringstream out;
    
    ASCII85::process(in, out, ASCII85::Mode::STREAM, false);
    std::string encoded = out.str();
    
    std::stringstream in2(encoded);
    std::stringstream out2;
    ASCII85::process(in2, out2, ASCII85::Mode::STREAM, true);
    
    EXPECT_EQ(out2.str(), input);
}

TEST(ASCII85Test, BufferMode) {
    std::string input = "Hello, World!";
    std::stringstream in(input);
    std::stringstream out;
    
    ASCII85::process(in, out, ASCII85::Mode::BUFFER, false);
    std::string encoded = out.str();
    
    std::stringstream in2(encoded);
    std::stringstream out2;
    ASCII85::process(in2, out2, ASCII85::Mode::BUFFER, true);
    
    EXPECT_EQ(out2.str(), input);
} 