#include "ascii85.hpp"
#include <iostream>
#include <string>
#include <cstring>

using namespace ascii85;

void printHelp() {
    std::cout << "Usage: ascii85 [options]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -e, --encode    Encode data (default)" << std::endl;
    std::cout << "  -d, --decode    Decode data" << std::endl;
    std::cout << "  -b, --buffer    Use buffer mode instead of stream mode" << std::endl;
    std::cout << "  -h, --help      Show this help message" << std::endl;
}

int main(int argc, char* argv[]) {
    ASCII85::Mode mode = ASCII85::Mode::STREAM;
    bool decode = false;
    
    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        const char* arg = argv[i];
        
        // Process arguments
        if (arg[0] == '-') {
            // Long option
            if (arg[1] == '-') {
                if (strcmp(arg, "--help") == 0) {
                    printHelp();
                    return 0;
                } else if (strcmp(arg, "--encode") == 0) {
                    decode = false;
                } else if (strcmp(arg, "--decode") == 0) {
                    decode = true;
                } else if (strcmp(arg, "--buffer") == 0) {
                    mode = ASCII85::Mode::BUFFER;
                } else {
                    std::cerr << "Unknown option: " << arg << std::endl;
                    return 1;
                }
            }
            // Short option
            else {
                
                for (int j = 1; arg[j] != '\0'; ++j) {
                    switch (arg[j]) {
                        case 'h':
                            printHelp();
                            return 0;
                        case 'e':
                            decode = false;
                            break;
                        case 'd':
                            decode = true;
                            break;
                        case 'b':
                            mode = ASCII85::Mode::BUFFER;
                            break;
                        default:
                            std::cerr << "Unknown option: -" << arg[j] << std::endl;
                            return 1;
                    }
                }
            }
        } else {
            std::cerr << "Unknown argument: " << arg << std::endl;
            return 1;
        }
    }
    
    try {
        ASCII85::process(std::cin, std::cout, mode, decode);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}