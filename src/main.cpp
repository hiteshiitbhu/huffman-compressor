#include <iostream>
#include <string>
#include "huffman.hpp"

void printUsage(const std::string& programName) {
    std::cout << "Usage:\n"
              << "  To Compress:\n"
              << "    " << programName << " -c <input_file> <output_file>\n\n"
              << "  To Decompress:\n"
              << "    " << programName << " -d <input_file> <output_file>\n";
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        printUsage(argv[0]);
        return 1;
    }

    std::string mode = argv[1];
    std::string inputPath = argv[2];
    std::string outputPath = argv[3];

    HuffmanCompressor compressor;

    try {
        if (mode == "-c") {
            std::cout << "Compressing file: " << inputPath << " -> " << outputPath << " ...\n";
            compressor.compress(inputPath, outputPath);
            std::cout << "Successfully compressed file!\n";
        } else if (mode == "-d") {
            std::cout << "Decompressing file: " << inputPath << " -> " << outputPath << " ...\n";
            compressor.decompress(inputPath, outputPath);
            std::cout << "Successfully decompressed file!\n";
        } else {
            std::cerr << "Error: Unknown flag '" << mode << "'\n\n";
            printUsage(argv[0]);
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
