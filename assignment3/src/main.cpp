#include "shannon_fano.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdexcept>
#include <cstring> 

// For Windows binary I/O for stdin/stdout
#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

/**
 * @brief Prints help information for the command-line tool.
 * @param appName The name of the executable.
 */
void printHelp(const char* appName) {
    std::cerr << "Shannon-Fano Coder/Decoder" << std::endl;
    std::cerr << "Usage: " << appName << " <mode> -t <dict_file> [-i <input_file>] [-o <output_file>]" << std::endl;
    std::cerr << "Modes:" << std::endl;
    std::cerr << "  -e, --encode        Encode data" << std::endl;
    std::cerr << "  -d, --decode        Decode data" << std::endl;
    std::cerr << "Options:" << std::endl;
    std::cerr << "  -t, --dict <file>   Dictionary file (required)" << std::endl;
    std::cerr << "  -i, --input <file>  Input file (default: stdin)" << std::endl;
    std::cerr << "  -o, --output <file> Output file (default: stdout)" << std::endl;
    std::cerr << "  -h, --help          Show this help message" << std::endl;
}

/**
 * @brief Main entry point for the Shannon-Fano command-line tool.
 */
int main(int argc, char* argv[]) {
    bool decode_mode = false;
    bool encode_mode = true;
    std::string input_filename;
    std::string output_filename;
    std::string dict_filename;

    if (argc <= 1) {
        printHelp(argv[0]);
        return 1;
    }

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            printHelp(argv[0]);
            return 0;
        } else if (arg == "-e" || arg == "--encode") {
            encode_mode = true;
            decode_mode = false;
        } else if (arg == "-d" || arg == "--decode") {
            decode_mode = true;
            encode_mode = false;
        } else if ((arg == "-i" || arg == "--input") && i + 1 < argc) {
            input_filename = argv[++i];
        } else if ((arg == "-o" || arg == "--output") && i + 1 < argc) {
            output_filename = argv[++i];
        } else if ((arg == "-t" || arg == "--dict") && i + 1 < argc) {
            dict_filename = argv[++i];
        } else {
            std::cerr << "Error: Unknown or incomplete option: " << arg << std::endl;
            printHelp(argv[0]);
            return 1;
        }
    }

    if (encode_mode && decode_mode) {
        std::cerr << "Error: Cannot specify both -e (encode) and -d (decode) modes simultaneously." << std::endl;
        printHelp(argv[0]);
        return 1;
    }
    if (dict_filename.empty()) {
        std::cerr << "Error: Dictionary file (-t or --dict) must be specified." << std::endl;
        printHelp(argv[0]);
        return 1;
    }

    std::istream* p_input = &std::cin;
    std::ostream* p_output = &std::cout;
    std::ios_base::openmode input_open_mode = std::ios_base::in | std::ios_base::binary;
    std::ios_base::openmode output_open_mode = std::ios_base::out | std::ios_base::binary | std::ios_base::trunc;

    std::ifstream ifs;
    std::ofstream ofs;
    std::fstream dict_file_stream;

    try {
        if (!input_filename.empty()) {
            ifs.open(input_filename, input_open_mode);
            if (!ifs.is_open()) {
                throw std::runtime_error("Failed to open input file: " + input_filename);
            }
            p_input = &ifs;
        } else {
            // Set stdin to binary mode if used
            #ifdef _WIN32
                if (_setmode(_fileno(stdin), _O_BINARY) == -1) {
                    throw std::runtime_error("Failed to set stdin to binary mode.");
                }
            #endif
        }

        if (!output_filename.empty()) {
            ofs.open(output_filename, output_open_mode);
            if (!ofs.is_open()) {
                throw std::runtime_error("Failed to open output file: " + output_filename);
            }
            p_output = &ofs;
        } else {
            // Set stdout to binary mode if used
            #ifdef _WIN32
                if (_setmode(_fileno(stdout), _O_BINARY) == -1) {
                    throw std::runtime_error("Failed to set stdout to binary mode.");
                }
            #endif
        }

        shannon_fano::ShannonFano sf_processor;

        if (decode_mode) {
            dict_file_stream.open(dict_filename, std::ios_base::in | std::ios_base::binary);
            if (!dict_file_stream.is_open()) {
                throw std::runtime_error("Failed to open dictionary file for reading: " + dict_filename);
            }
            std::cerr << "Decoding..." << std::endl;
            sf_processor.decode(*p_input, dict_file_stream, *p_output);
            std::cerr << "Decoding completed." << std::endl;
        } else {
            dict_file_stream.open(dict_filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
            if (!dict_file_stream.is_open()) {
                throw std::runtime_error("Failed to open dictionary file for writing: " + dict_filename);
            }
            std::cerr << "Encoding..." << std::endl;
            sf_processor.encode(*p_input, *p_output, dict_file_stream);
            std::cerr << "Encoding completed." << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        
        if (ifs.is_open()) ifs.close();
        if (ofs.is_open()) ofs.close();
        if (dict_file_stream.is_open()) dict_file_stream.close();
        return 1;
    }

    if (ifs.is_open()) ifs.close();
    if (ofs.is_open()) ofs.close();
    if (dict_file_stream.is_open()) dict_file_stream.close();
    
    return 0;
}