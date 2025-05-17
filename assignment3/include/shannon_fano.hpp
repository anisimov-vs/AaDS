#ifndef SHANNON_FANO_HPP
#define SHANNON_FANO_HPP

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <cstdint>
#include <algorithm>
#include <numeric>

namespace shannon_fano {

struct TrieNode; // Forward declaration

/**
 * @brief Implements the Shannon-Fano compression and decompression algorithm.
 */
class ShannonFano {
public:
    ShannonFano();
    ~ShannonFano();

    /**
     * @brief Encodes data from an input stream and writes the compressed data and dictionary.
     * @param inputFile Stream to read uncompressed data from.
     * @param outputFile Stream to write compressed data to.
     * @param dictFile Stream to write the Shannon-Fano dictionary to.
     * @throws std::runtime_error on I/O errors or internal processing failures.
     */
    void encode(std::istream& inputFile, std::ostream& outputFile, std::ostream& dictFile);

    /**
     * @brief Decodes data from an input stream using a dictionary file.
     * @param inputFile Stream to read compressed data from.
     * @param dictFile Stream to read the Shannon-Fano dictionary from.
     * @param outputFile Stream to write decompressed data to.
     * @throws std::runtime_error on I/O errors, dictionary errors, or data corruption.
     */
    void decode(std::istream& inputFile, std::istream& dictFile, std::ostream& outputFile);

private:
    std::map<unsigned char, size_t> frequencyMap;
    std::map<unsigned char, std::string> codeTable;
    TrieNode* decodeTrieRoot;
    uint64_t originalFileSize;

    struct SymbolInfo {
        unsigned char symbol;
        size_t frequency;
        SymbolInfo(unsigned char s, size_t f) : symbol(s), frequency(f) {}
        bool operator<(const SymbolInfo& other) const {
            if (frequency != other.frequency) return frequency > other.frequency;
            return symbol < other.symbol;
        }
    };

    void clearState();

    void buildFrequencyMap(std::istream& inputFile);
    void buildCodesInternal();
    void shannonFanoRecursive(std::vector<SymbolInfo>::iterator begin,
                              std::vector<SymbolInfo>::iterator end,
                              const std::string& currentCode);
    void writeDictionary(std::ostream& dictFile);
    void writeCompressedData(std::istream& inputFile, std::ostream& outputFile);

    void readDictionary(std::istream& dictFile);
    void buildDecodingTrie();
    void readCompressedDataAndDecode(std::istream& inputFile, std::ostream& outputFile);

    void insertIntoTrie(TrieNode* root, const std::string& code, unsigned char symbol);
    void clearTrie(TrieNode* node);

    /**
     * @brief Helper class for writing individual bits to an output stream.
     */
    class BitWriter {
        std::ostream& os;
        unsigned char buffer;
        int bit_count;
    public:
        BitWriter(std::ostream& o);
        void writeBit(bool bit);
        void flush();
    };

    /**
     * @brief Helper class for reading individual bits from an input stream.
     */
    class BitReader {
        std::istream& is;
        unsigned char buffer;
        int bit_count;
    public:
        BitReader(std::istream& i);
        bool readBit(bool& bit_val);
    };
};

/**
 * @brief Node for the decoding Trie (prefix tree).
 */
struct TrieNode {
    std::map<char, TrieNode*> children; // '0' or '1'
    bool isEndOfCode;
    unsigned char symbol; // Decoded symbol if isEndOfCode is true
    TrieNode() : isEndOfCode(false), symbol(0) {}
};

} // namespace shannon_fano

#endif // SHANNON_FANO_HPP