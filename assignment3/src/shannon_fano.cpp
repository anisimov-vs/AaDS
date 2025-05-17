#include "shannon_fano.hpp"
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <numeric>
#include <sstream>

namespace shannon_fano {

ShannonFano::ShannonFano() : decodeTrieRoot(nullptr), originalFileSize(0) {}

ShannonFano::~ShannonFano() {
    clearTrie(decodeTrieRoot);
}

void ShannonFano::clearState() {
    frequencyMap.clear();
    codeTable.clear();
    clearTrie(decodeTrieRoot);
    decodeTrieRoot = nullptr;
    originalFileSize = 0;
}

ShannonFano::BitWriter::BitWriter(std::ostream& o) : os(o), buffer(0), bit_count(0) {}

void ShannonFano::BitWriter::writeBit(bool bit) {
    buffer = (buffer << 1) | (bit ? 1 : 0);
    bit_count++;
    if (bit_count == 8) {
        os.put(static_cast<char>(buffer));
        if (!os.good()) throw std::runtime_error("Failed to write byte to output stream.");
        buffer = 0;
        bit_count = 0;
    }
}

void ShannonFano::BitWriter::flush() {
    if (bit_count > 0) {
        buffer <<= (8 - bit_count); // Pad with 0s
        os.put(static_cast<char>(buffer));
        if (!os.good()) throw std::runtime_error("Failed to write final byte to output stream during flush.");
        buffer = 0;
        bit_count = 0;
    }
    os.flush();
}

ShannonFano::BitReader::BitReader(std::istream& i) : is(i), buffer(0), bit_count(0) {}

bool ShannonFano::BitReader::readBit(bool& bit_val) {
    if (bit_count == 0) {
        int byte_val = is.get();
        if (byte_val == EOF) return false;
        buffer = static_cast<unsigned char>(byte_val);
        bit_count = 8;
    }
    bit_val = (buffer & 0x80) != 0; // MSB
    buffer <<= 1;
    bit_count--;
    return true;
}

/*void ShannonFano::encode(std::istream& inputFile, std::ostream& outputFile, std::ostream& dictFile) {
    clearState();

    if (!inputFile.good()) throw std::runtime_error("Input file stream is not good before encoding.");
    if (!outputFile.good()) throw std::runtime_error("Output file stream is not good before encoding.");
    if (!dictFile.good()) throw std::runtime_error("Dictionary file stream is not good before encoding.");

    std::streampos initialPos = inputFile.tellg();
    buildFrequencyMap(inputFile);

    if (frequencyMap.empty()) {
        writeDictionary(dictFile);
        return;
    }
    
    buildCodesInternal();
    writeDictionary(dictFile);

    inputFile.clear(); 
    inputFile.seekg(initialPos); 
    if (!inputFile.good()) {
         throw std::runtime_error("Input file stream could not be reset for encoding pass. Consider buffering for non-seekable streams.");
    }
    writeCompressedData(inputFile, outputFile);
}*/

// shannon_fano.cpp
// ... inside ShannonFano::encode method ...
void ShannonFano::encode(std::istream& inputFile, std::ostream& outputFile, std::ostream& dictFile) {
    clearState();

    if (!outputFile.good()) throw std::runtime_error("Output file stream is not good before encoding.");
    if (!dictFile.good()) throw std::runtime_error("Dictionary file stream is not good before encoding.");

    std::istream* pStreamToProcess = &inputFile;
    std::stringstream bufferedInput;
    
    inputFile.clear(); // Clear any error flags
    std::streampos initialPosCheck = inputFile.tellg();
    inputFile.seekg(0, std::ios_base::end); // Try to seek to end
    bool isSeekable = inputFile.good();
    inputFile.clear(); // Clear flags from seek attempt
    inputFile.seekg(initialPosCheck); // Go back to original position if possible


    if (!isSeekable || (&inputFile == &std::cin) ) { // Added explicit check for std::cin for robustness
        // Buffer std::cin or other non-seekable streams
        bufferedInput << inputFile.rdbuf(); // Read entire stream into stringstream
        if (inputFile.bad()) { // Check if reading from original inputFile failed
             throw std::runtime_error("Failed to read from input stream into buffer.");
        }
        pStreamToProcess = &bufferedInput; // Process the buffered data
        // Note: original inputFile is now exhausted if it was a pipe
    }

    // --- Pass 1: Build Frequency Map ---
    // Ensure the stream to process is at the beginning for the first pass
    pStreamToProcess->clear();
    pStreamToProcess->seekg(0, std::ios_base::beg);
    if (!pStreamToProcess->good()) {
        throw std::runtime_error("Could not seek to beginning of processing stream for frequency map.");
    }
    buildFrequencyMap(*pStreamToProcess);

    if (frequencyMap.empty()) {
        writeDictionary(dictFile);
        return;
    }
    
    buildCodesInternal();
    writeDictionary(dictFile);

    // --- Pass 2: Write Compressed Data ---
    // Ensure the stream to process is at the beginning for the second pass
    pStreamToProcess->clear();
    pStreamToProcess->seekg(0, std::ios_base::beg);
     if (!pStreamToProcess->good()) {
        throw std::runtime_error("Could not seek to beginning of processing stream for compression pass.");
    }
    writeCompressedData(*pStreamToProcess, outputFile);
}

void ShannonFano::buildFrequencyMap(std::istream& inputFile) {
    frequencyMap.clear();
    originalFileSize = 0;
    char byte_char;
    while (inputFile.get(byte_char)) {
        frequencyMap[static_cast<unsigned char>(byte_char)]++;
        originalFileSize++;
    }
    inputFile.clear();
}

void ShannonFano::buildCodesInternal() {
    codeTable.clear();
    std::vector<SymbolInfo> symbols;
    for (const auto& pair : frequencyMap) {
        symbols.emplace_back(pair.first, pair.second);
    }

    if (symbols.empty()) return;

    std::sort(symbols.begin(), symbols.end());

    if (symbols.size() == 1) {
        codeTable[symbols[0].symbol] = "0"; // Single symbol gets code "0"
    } else {
        shannonFanoRecursive(symbols.begin(), symbols.end(), "");
    }
}

void ShannonFano::shannonFanoRecursive(std::vector<SymbolInfo>::iterator begin,
                                       std::vector<SymbolInfo>::iterator end,
                                       const std::string& currentCode) {
    auto num_symbols_in_range = std::distance(begin, end);

    if (num_symbols_in_range == 0) return;
    if (num_symbols_in_range == 1) {
        codeTable[begin->symbol] = currentCode;
        return;
    }

    size_t total_freq_in_range = 0;
    for (auto it = begin; it != end; ++it) total_freq_in_range += it->frequency;

    size_t current_sum_freq = 0;
    auto split_iter = begin;
    size_t min_diff = total_freq_in_range;

    for (auto it_try_split = begin; it_try_split != std::prev(end); ++it_try_split) {
        current_sum_freq += it_try_split->frequency;
        size_t diff = std::abs(static_cast<long long>(current_sum_freq) - static_cast<long long>(total_freq_in_range - current_sum_freq));
        if (diff < min_diff) {
            min_diff = diff;
            split_iter = std::next(it_try_split);
        } else if (diff >= min_diff && current_sum_freq > (total_freq_in_range / 2)) {
            break; 
        }
    }
    
    if(split_iter == begin && num_symbols_in_range > 1) { // Ensure split point advances if possible
        split_iter = std::next(begin);
    }

    shannonFanoRecursive(begin, split_iter, currentCode + "0");
    shannonFanoRecursive(split_iter, end, currentCode + "1");
}

void ShannonFano::writeDictionary(std::ostream& dictFile) {
    dictFile.write(reinterpret_cast<const char*>(&originalFileSize), sizeof(originalFileSize));

    uint16_t num_entries = static_cast<uint16_t>(codeTable.size());
    dictFile.write(reinterpret_cast<const char*>(&num_entries), sizeof(num_entries));

    for (const auto& pair : codeTable) {
        unsigned char symbol = pair.first;
        const std::string& code_str = pair.second;
        uint8_t code_length = static_cast<uint8_t>(code_str.length());

        dictFile.put(static_cast<char>(symbol));
        dictFile.put(static_cast<char>(code_length));
        dictFile.write(code_str.data(), code_length);
    }
    if (!dictFile.good()) throw std::runtime_error("Failed to write dictionary to stream.");
    dictFile.flush();
}

void ShannonFano::writeCompressedData(std::istream& inputFile, std::ostream& outputFile) {
    BitWriter writer(outputFile);
    char byte_char;
    uint64_t bytes_processed = 0;

    while (bytes_processed < originalFileSize && inputFile.get(byte_char)) {
        const std::string& code = codeTable.at(static_cast<unsigned char>(byte_char)); 
        for (char bit_char : code) writer.writeBit(bit_char == '1');
        bytes_processed++;
    }
    
    if (bytes_processed != originalFileSize && originalFileSize != 0) {
        throw std::runtime_error("Mismatch in file size during compression pass.");
    }
    writer.flush();
}

void ShannonFano::decode(std::istream& inputFile, std::istream& dictFile, std::ostream& outputFile) {
    clearState();

    if (!inputFile.good()) throw std::runtime_error("Input file stream is not good before decoding.");
    if (!dictFile.good()) throw std::runtime_error("Dictionary file stream is not good before decoding.");
    if (!outputFile.good()) throw std::runtime_error("Output file stream is not good before decoding.");

    readDictionary(dictFile);

    if (originalFileSize == 0) {
        outputFile.flush();
        return;
    }
    
    buildDecodingTrie(); 
    if (!decodeTrieRoot) throw std::runtime_error("Decoding Trie was not built.");

    readCompressedDataAndDecode(inputFile, outputFile);
    outputFile.flush();
}

void ShannonFano::readDictionary(std::istream& dictFile) {
    dictFile.read(reinterpret_cast<char*>(&originalFileSize), sizeof(originalFileSize));
    if (dictFile.gcount() != sizeof(originalFileSize)) throw std::runtime_error("Failed to read original file size from dictionary.");

    uint16_t num_entries;
    dictFile.read(reinterpret_cast<char*>(&num_entries), sizeof(num_entries));
    if (dictFile.gcount() != sizeof(num_entries)) throw std::runtime_error("Failed to read number of entries from dictionary.");

    codeTable.clear();
    for (uint16_t i = 0; i < num_entries; ++i) {
        unsigned char symbol = static_cast<unsigned char>(dictFile.get());
        if (dictFile.eof()) throw std::runtime_error("Unexpected EOF while reading symbol from dictionary.");
        
        uint8_t code_length = static_cast<uint8_t>(dictFile.get());
        if (dictFile.eof()) throw std::runtime_error("Unexpected EOF while reading code length from dictionary.");
        
        if (code_length == 0 && (num_entries > 1 || (num_entries == 1 && originalFileSize > 0))) {
             throw std::runtime_error("Invalid zero code length in dictionary.");
        }

        std::string code_str(code_length, '\0');
        if (code_length > 0) {
             dictFile.read(&code_str[0], code_length);
             if (dictFile.gcount() != code_length) throw std::runtime_error("Failed to read code string from dictionary.");
        }
        codeTable[symbol] = code_str;
    }
    if (!dictFile.good() && !dictFile.eof()) throw std::runtime_error("Error reading dictionary stream after entries.");
}

void ShannonFano::buildDecodingTrie() {
    clearTrie(decodeTrieRoot);
    decodeTrieRoot = new TrieNode();

    if (codeTable.empty() && originalFileSize > 0) {
        throw std::runtime_error("Code table is empty for a non-empty file during Trie construction.");
    }
    for (const auto& pair : codeTable) {
        insertIntoTrie(decodeTrieRoot, pair.second, pair.first);
    }
}

void ShannonFano::readCompressedDataAndDecode(std::istream& inputFile, std::ostream& outputFile) {
    BitReader reader(inputFile);
    TrieNode* currentNode = decodeTrieRoot;
    uint64_t decodedBytesCount = 0;

    bool bit_val;
    while (decodedBytesCount < originalFileSize && reader.readBit(bit_val)) {
        char bit_char = bit_val ? '1' : '0';
        auto it = currentNode->children.find(bit_char);
        if (it == currentNode->children.end()) throw std::runtime_error("Invalid bit sequence in compressed data: no path in Trie.");
        
        currentNode = it->second;
        if (currentNode->isEndOfCode) {
            outputFile.put(static_cast<char>(currentNode->symbol));
            if (!outputFile.good()) throw std::runtime_error("Failed to write decoded byte to output stream.");
            decodedBytesCount++;
            currentNode = decodeTrieRoot;
        }
    }

    if (decodedBytesCount != originalFileSize) {
        throw std::runtime_error("Decoding failed: Decoded bytes do not match original file size. Input may be truncated/corrupt.");
    }
}

void ShannonFano::insertIntoTrie(TrieNode* root, const std::string& code, unsigned char symbol) {
    TrieNode* current = root;
    for (char bit_char : code) {
        if (current->children.find(bit_char) == current->children.end()) {
            current->children[bit_char] = new TrieNode();
        }
        current = current->children[bit_char];
    }
    current->isEndOfCode = true;
    current->symbol = symbol;
}

void ShannonFano::clearTrie(TrieNode* node) {
    if (!node) return;
    for (auto& pair : node->children) clearTrie(pair.second);
    delete node;
}

} // namespace shannon_fano