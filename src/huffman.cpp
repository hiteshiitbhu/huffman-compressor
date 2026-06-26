#include "huffman.hpp"
#include <queue>
#include <stdexcept>
#include <vector>

void HuffmanCompressor::buildFrequencyMap(std::ifstream& input, std::unordered_map<char, uint64_t>& freqMap, uint64_t& totalSize) {
    char ch;
    totalSize = 0;
    freqMap.clear();
    while (input.get(ch)) {
        freqMap[ch]++;
        totalSize++;
    }
}

HuffmanNode* HuffmanCompressor::buildTree(const std::unordered_map<char, uint64_t>& freqMap) {
    std::priority_queue<HuffmanNode*, std::vector<HuffmanNode*>, CompareNode> pq;
    
    for (const auto& pair : freqMap) {
        pq.push(new HuffmanNode(pair.first, pair.second));
    }
    
    if (pq.empty()) {
        return nullptr;
    }
    
    // Edge case: if there's only 1 unique character in the file
    if (pq.size() == 1) {
        HuffmanNode* singleNode = pq.top();
        pq.pop();
        return new HuffmanNode(singleNode->freq, singleNode, nullptr);
    }
    
    while (pq.size() > 1) {
        HuffmanNode* left = pq.top(); pq.pop();
        HuffmanNode* right = pq.top(); pq.pop();
        HuffmanNode* parent = new HuffmanNode(left->freq + right->freq, left, right);
        pq.push(parent);
    }
    
    return pq.top();
}

void HuffmanCompressor::generateCodes(HuffmanNode* node, const std::string& currentCode, std::unordered_map<char, std::string>& codes) {
    if (!node) return;
    
    // Leaf node check
    if (!node->left && !node->right) {
        codes[node->ch] = currentCode.empty() ? "0" : currentCode;
        return;
    }
    
    generateCodes(node->left, currentCode + "0", codes);
    generateCodes(node->right, currentCode + "1", codes);
}

void HuffmanCompressor::writeHeader(std::ofstream& output, uint64_t totalSize, const std::unordered_map<char, uint64_t>& freqMap) {
    // 1. Original file size (8 bytes)
    output.write(reinterpret_cast<const char*>(&totalSize), sizeof(totalSize));
    
    // 2. Number of unique characters (2 bytes)
    uint16_t mapSize = static_cast<uint16_t>(freqMap.size());
    output.write(reinterpret_cast<const char*>(&mapSize), sizeof(mapSize));
    
    // 3. Frequency table: list of (char, uint64_t)
    for (const auto& pair : freqMap) {
        char ch = pair.first;
        uint64_t freq = pair.second;
        output.write(&ch, sizeof(ch));
        output.write(reinterpret_cast<const char*>(&freq), sizeof(freq));
    }
}

void HuffmanCompressor::readHeader(std::ifstream& input, uint64_t& totalSize, std::unordered_map<char, uint64_t>& freqMap) {
    freqMap.clear();
    
    // 1. Read original size
    input.read(reinterpret_cast<char*>(&totalSize), sizeof(totalSize));
    
    // 2. Read number of unique characters
    uint16_t mapSize = 0;
    input.read(reinterpret_cast<char*>(&mapSize), sizeof(mapSize));
    
    // 3. Read frequency map entries
    for (uint16_t i = 0; i < mapSize; ++i) {
        char ch;
        uint64_t freq;
        input.read(&ch, sizeof(ch));
        input.read(reinterpret_cast<char*>(&freq), sizeof(freq));
        freqMap[ch] = freq;
    }
}

void HuffmanCompressor::compress(const std::string& inputPath, const std::string& outputPath) {
    std::ifstream input(inputPath, std::ios::binary);
    if (!input) {
        throw std::runtime_error("Could not open input file: " + inputPath);
    }
    
    std::unordered_map<char, uint64_t> freqMap;
    uint64_t totalSize = 0;
    buildFrequencyMap(input, freqMap, totalSize);
    
    std::ofstream output(outputPath, std::ios::binary);
    if (!output) {
        throw std::runtime_error("Could not open output file: " + outputPath);
    }
    
    if (totalSize == 0) {
        writeHeader(output, 0, freqMap);
        return;
    }
    
    HuffmanNode* root = buildTree(freqMap);
    std::unordered_map<char, std::string> codes;
    generateCodes(root, "", codes);
    
    writeHeader(output, totalSize, freqMap);
    
    // Reset file to beginning to read data characters
    input.clear();
    input.seekg(0, std::ios::beg);
    
    unsigned char bitBuffer = 0;
    int bitCount = 0;
    char ch;
    
    while (input.get(ch)) {
        const std::string& code = codes[ch];
        for (char bitChar : code) {
            bitBuffer = (bitBuffer << 1) | (bitChar == '1' ? 1 : 0);
            bitCount++;
            if (bitCount == 8) {
                output.put(static_cast<char>(bitBuffer));
                bitBuffer = 0;
                bitCount = 0;
            }
        }
    }
    
    // Pad remaining bits with zeros if necessary
    if (bitCount > 0) {
        bitBuffer <<= (8 - bitCount);
        output.put(static_cast<char>(bitBuffer));
    }
    
    delete root;
}

void HuffmanCompressor::decompress(const std::string& inputPath, const std::string& outputPath) {
    std::ifstream input(inputPath, std::ios::binary);
    if (!input) {
        throw std::runtime_error("Could not open input file: " + inputPath);
    }
    
    uint64_t totalSize = 0;
    std::unordered_map<char, uint64_t> freqMap;
    readHeader(input, totalSize, freqMap);
    
    std::ofstream output(outputPath, std::ios::binary);
    if (!output) {
        throw std::runtime_error("Could not open output file: " + outputPath);
    }
    
    if (totalSize == 0) {
        return; // Empty file decompression complete
    }
    
    HuffmanNode* root = buildTree(freqMap);
    HuffmanNode* current = root;
    uint64_t decodedCount = 0;
    char byteChar;
    
    while (decodedCount < totalSize && input.get(byteChar)) {
        unsigned char byte = static_cast<unsigned char>(byteChar);
        for (int i = 7; i >= 0; --i) {
            int bit = (byte >> i) & 1;
            if (bit == 0) {
                current = current->left;
            } else {
                current = current->right;
            }
            
            // Reached leaf node
            if (!current->left && !current->right) {
                output.put(current->ch);
                decodedCount++;
                current = root; // Reset traversal to root
                if (decodedCount == totalSize) {
                    break;
                }
            }
        }
    }
    
    delete root;
}
