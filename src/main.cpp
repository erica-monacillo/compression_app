#include <iostream>
#include <fstream>
#include <cmath>
#include <unordered_map>
#include <vector>
#include <string>
#include "dwt_db4.hpp"
#include "huffman.hpp"
#include "image_io.hpp"
#include "utils.hpp"

// Detect image size (assumes square of float pixels). Returns 0 on success, -1 on failure
int detectSize(const std::string& path, int& rows, int& cols) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) {
        std::cerr << "Error: Cannot open file " << path << std::endl;
        return -1;
    }
    std::streamsize size = file.tellg();
    file.close();

    if (size % sizeof(float) != 0) {
        std::cerr << "Warning: File size is not a multiple of sizeof(float) in " << path << std::endl;
        return -1;
    }

    int pixelCount = size / sizeof(float);
    int dim = static_cast<int>(std::sqrt(pixelCount));
    if (dim * dim != pixelCount) {
        std::cerr << "Warning: Detected image size is not square in " << path << std::endl;
        return -1;
    }

    rows = cols = dim;
    return 0;
}

// Ensure the 2D vector has even dimensions by padding last row and column if necessary
void padToEven(std::vector<std::vector<float>>& image) {
    if (image.empty()) return;
    size_t max_cols = 0;
    for (const auto& row : image) max_cols = std::max(max_cols, row.size());

    // Pad all rows to max_cols
    for (auto& row : image) {
        while (row.size() < max_cols) row.push_back(row.back());
    }

    // Pad rows if needed
    if (image.size() % 2 != 0) {
        image.push_back(image.back());
    }
    // Pad columns if needed
    if (max_cols % 2 != 0) {
        for (auto& row : image) {
            row.push_back(row.back());
        }
    }
}

// Utility: check all rows are the same size
bool checkRowSizes(const std::vector<std::vector<float>>& img, const std::string& label) {
    if (img.empty()) return true;
    size_t expected = img[0].size();
    for (size_t i = 0; i < img.size(); ++i) {
        if (img[i].size() != expected) {
            std::cerr << label << " Row " << i << " size " << img[i].size()
                      << " does not match expected " << expected << std::endl;
            return false;
        }
    }
    return true;
}

int main() {
    std::string outputPath = "output/reconstructed_image.png";
    std::string bandPaths[3] = {
        "data/band_0.bin",
        "data/band_1.bin",
        "data/band_2.bin"
    };

    int rows = 0, cols = 0;
    if (detectSize(bandPaths[0], rows, cols) != 0) {
        std::cerr << "❌ Failed to detect size from input band." << std::endl;
        return -1;
    }
    std::cout << "[INFO] Detected image size: " << rows << "x" << cols << std::endl;

    std::cout << "[1] Loading raw hyperspectral bands..." << std::endl;
    std::vector<std::vector<float>> R = loadBinImage(bandPaths[0], rows, cols);
    std::vector<std::vector<float>> G = loadBinImage(bandPaths[1], rows, cols);
    std::vector<std::vector<float>> B = loadBinImage(bandPaths[2], rows, cols);

    // Check loaded images sizes consistency
    if (R.size() != (size_t)rows || G.size() != (size_t)rows || B.size() != (size_t)rows ||
        R[0].size() != (size_t)cols || G[0].size() != (size_t)cols || B[0].size() != (size_t)cols) {
        std::cerr << "❌ Loaded images have inconsistent sizes." << std::endl;
        return -1;
    }

    std::vector<std::vector<std::vector<float>>> channels = {R, G, B};
    std::vector<std::vector<std::vector<float>>> channels_reconstructed;

    for (int c = 0; c < 3; ++c) {
        std::cout << "\n=== Processing Channel " << c << " ===" << std::endl;
        auto image = channels[c];

        std::cout << "[DEBUG] Original image size: " << image.size() << " x " << image[0].size() << std::endl;
        padToEven(image);
        if (!checkRowSizes(image, "Image")) return -1;
        std::cout << "[DEBUG] Padded image size: " << image.size() << " x " << image[0].size() << std::endl;

        // Level 1 DWT
        padToEven(image); // Ensure even before DWT
        if (!checkRowSizes(image, "Image before Level 1 DWT")) return -1;
        std::vector<std::vector<float>> LL1, LH1, HL1, HH1;
        dwt2D_db4(image, LL1, LH1, HL1, HH1);
        if (LL1.empty() || LL1[0].empty()) {
            std::cerr << "❌ Error: LL1 is empty after DWT!" << std::endl;
            return -1;
        }

        // Pad all subbands after first DWT
        padToEven(LL1);
        padToEven(LH1);
        padToEven(HL1);
        padToEven(HH1);
        if (!checkRowSizes(LL1, "LL1 after padding")) return -1;
        if (!checkRowSizes(LH1, "LH1 after padding")) return -1;
        if (!checkRowSizes(HL1, "HL1 after padding")) return -1;
        if (!checkRowSizes(HH1, "HH1 after padding")) return -1;
        std::cout << "[DEBUG] LL1 size after padding: "
                  << LL1.size() << "x" << (LL1.empty() ? 0 : LL1[0].size()) << std::endl;

        // Level 2 DWT
        padToEven(LL1); // Ensure even before DWT
        if (!checkRowSizes(LL1, "LL1 before Level 2 DWT")) return -1;
        std::vector<std::vector<float>> LL2, LH2, HL2, HH2;
        dwt2D_db4(LL1, LL2, LH2, HL2, HH2);
        if (LL2.empty() || LL2[0].empty()) {
            std::cerr << "❌ Error: LL2 is empty after DWT!" << std::endl;
            return -1;
        }
        std::cout << "[DEBUG] LL2 size: " << LL2.size() << " x " << LL2[0].size() << std::endl;

        // Flatten LL2 for encoding
        std::cout << "[3] Flattening + Real Huffman Encoding..." << std::endl;
        std::vector<int> flattened = flatten(LL2);
        if (flattened.empty()) {
            std::cerr << "❌ Flattened LL2 data is empty!" << std::endl;
            return -1;
        }

        std::unordered_map<int, std::string> huffTable;
        std::string encoded = huffmanEncode(flattened, huffTable);

        // Check if huffTable and encoded data are valid
        if (encoded.empty() || huffTable.empty()) {
            std::cerr << "❌ Huffman encoding failed or produced empty data." << std::endl;
            return -1;
        }

        // Build reverse lookup map code->value
        std::unordered_map<std::string, int> reverseTable;
        bool duplicatesFound = false;
        for (const auto& [val, code] : huffTable) {
            if (reverseTable.count(code)) {
                std::cerr << "Warning: Duplicate Huffman code found: " << code << std::endl;
                duplicatesFound = true;
            }
            reverseTable[code] = val;
        }
        if (duplicatesFound) {
            std::cerr << "Warning: Huffman table contains duplicate codes! Decoding might fail." << std::endl;
        }

        std::vector<int> decoded = huffmanDecode(encoded, reverseTable);
        size_t expectedSize = LL2.size() * (LL2.empty() ? 0 : LL2[0].size());
        if (decoded.size() < expectedSize) {
            std::cerr << "❌ Error: Decoded Huffman data too short! Expected " << expectedSize << ", got " << decoded.size() << std::endl;
            return -1;
        }
        if (decoded.size() > expectedSize) {
            std::cout << "Warning: Decoded Huffman data longer than expected. Truncating." << std::endl;
            decoded.resize(expectedSize);
        }

        std::vector<std::vector<float>> reconstructed_LL2 = unflatten(decoded, LL2.size(), LL2[0].size());

        std::cout << "[4] Reconstructing..." << std::endl;

        // Use the actual subbands for reconstruction!
        std::vector<std::vector<float>> reconstructed_LL1 = idwt2D_db4(
            reconstructed_LL2, LH2, HL2, HH2);

        std::vector<std::vector<float>> reconstructed = idwt2D_db4(
            reconstructed_LL1, LH1, HL1, HH1);

        // Print and normalize value range before saving
        float minVal = reconstructed[0][0], maxVal = reconstructed[0][0];
        for (const auto& row : reconstructed)
            for (float v : row) {
                if (v < minVal) minVal = v;
                if (v > maxVal) maxVal = v;
            }
        std::cout << "[DEBUG] Reconstructed min: " << minVal << " max: " << maxVal << std::endl;
        if (maxVal > minVal && (minVal < 0.0f || maxVal > 255.0f)) {
            std::cout << "[DEBUG] Normalizing reconstructed channel to [0,255]" << std::endl;
            for (auto& row : reconstructed)
                for (float& v : row)
                    v = 255.0f * (v - minVal) / (maxVal - minVal);
        }

        std::cout << "[5] Evaluating..." << std::endl;
        evaluate(image, reconstructed);
        double ssim = computeSSIM(image, reconstructed);
        std::cout << "SSIM: " << ssim << std::endl;

        double originalSize = static_cast<double>(flattened.size()) * sizeof(int);
        double compressedSize = static_cast<double>(encoded.size()) / 8.0; // bits to bytes
        double cr = compressedSize > 0.0 ? originalSize / compressedSize : 0.0;
        double bpp = (compressedSize * 8.0) / (image.size() * image[0].size());

        std::cout << "Compression Ratio (CR): " << cr << std::endl;
        std::cout << "Bits Per Pixel (BPP): " << bpp << std::endl;

        // Save Huffman encoded bin file
        std::string binFile = "output/encoded_band_" + std::to_string(c) + ".bin";
        std::ofstream out(binFile, std::ios::binary);
        if (!out) {
            std::cerr << "Error: Could not open " << binFile << " for writing." << std::endl;
            return -1;
        }
        out.write(encoded.c_str(), static_cast<std::streamsize>(encoded.size()));
        out.close();

        channels_reconstructed.push_back(std::move(reconstructed));
    }

    std::cout << "[6] Saving full color output..." << std::endl;
    if (channels_reconstructed.size() == 3) {
        saveColorImage(channels_reconstructed[0], channels_reconstructed[1], channels_reconstructed[2], outputPath);
        std::cout << "✅ DONE! Output saved to " << outputPath << std::endl;
    } else {
        std::cerr << "❌ Error: Not all channels were reconstructed." << std::endl;
        return -1;
    }

    return 0;
}