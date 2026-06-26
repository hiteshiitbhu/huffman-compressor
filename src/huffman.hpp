#ifndef HUFFMAN_HPP
#define HUFFMAN_HPP

#include <string>
#include <unordered_map>
#include <cstdint>
#include <fstream>

#include <algorithm>

struct HuffmanNode {
    char ch;
    uint64_t freq;
    char min_char;
    HuffmanNode* left;
    HuffmanNode* right;

    HuffmanNode(char c, uint64_t f) : ch(c), freq(f), min_char(c), left(nullptr), right(nullptr) {}
    HuffmanNode(uint64_t f, HuffmanNode* l, HuffmanNode* r) : ch('\0'), freq(f), left(l), right(r) {
        unsigned char leftMin = l ? static_cast<unsigned char>(l->min_char) : 255;
        unsigned char rightMin = r ? static_cast<unsigned char>(r->min_char) : 255;
        min_char = static_cast<char>(std::min(leftMin, rightMin));
    }
    ~HuffmanNode() {
        delete left;
        delete right;
    }
};

// Comparator for the min-priority queue (fully deterministic)
struct CompareNode {
    bool operator()(const HuffmanNode* a, const HuffmanNode* b) const {
        if (a->freq != b->freq) {
            return a->freq > b->freq; // Min-heap behavior
        }
        return static_cast<unsigned char>(a->min_char) > static_cast<unsigned char>(b->min_char);
    }
};

class HuffmanCompressor {
public:
    HuffmanCompressor() = default;
    ~HuffmanCompressor() = default;

    // Compresses inputPath to outputPath
    void compress(const std::string& inputPath, const std::string& outputPath);

    // Decompresses inputPath to outputPath
    void decompress(const std::string& inputPath, const std::string& outputPath);

private:
    // Helper functions for compression
    void buildFrequencyMap(std::ifstream& input, std::unordered_map<char, uint64_t>& freqMap, uint64_t& totalSize);
    HuffmanNode* buildTree(const std::unordered_map<char, uint64_t>& freqMap);
    void generateCodes(HuffmanNode* node, const std::string& currentCode, std::unordered_map<char, std::string>& codes);
    void writeHeader(std::ofstream& output, uint64_t totalSize, const std::unordered_map<char, uint64_t>& freqMap);

    // Helper functions for decompression
    void readHeader(std::ifstream& input, uint64_t& totalSize, std::unordered_map<char, uint64_t>& freqMap);
};

#endif // HUFFMAN_HPP
