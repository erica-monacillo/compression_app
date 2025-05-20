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

// Function to detect image size from a binary file (returns 0 on success, -1 on failure)
int detectSize(const std::string& filename, int& rows, int& cols) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file) return -1;
    std::streamsize size = file.tellg();
    file.close();
    if (size % sizeof(float) != 0) return -1;
    int pixelCount = size / sizeof(float);
    int dim = static_cast<int>(std::sqrt(pixelCount));
    if (dim * dim != pixelCount) return -1;
    rows = cols = dim;
    return 0;
}

// Pads a 2D vector to even dimensions by duplicating the last row/column if needed
void padToEven(std::vector<std::vector<float>>& img) {
    if (img.empty()) return;
    // Pad rows
    if (img.size() % 2 != 0) {
        img.push_back(img.back());
    }
    // Pad columns
    size_t cols = img[0].size();
    if (cols % 2 != 0) {
        for (auto& row : img) {
            row.push_back(row.back());
        }
    }
}

// Checks that all rows in a 2D vector have the same size; prints error if not
bool checkRowSizes(const std::vector<std::vector<float>>& img, const std::string& name) {
    if (img.empty()) return true;
    size_t cols = img[0].size();
    for (size_t i = 0; i < img.size(); ++i) {
        if (img[i].size() != cols) {
            std::cerr << "❌ Error: " << name << " row " << i << " has size " << img[i].size()
                      << " (expected " << cols << ")" << std::endl;
            return false;
        }
    }
    return true;
}

// Quantize a subband in-place
void quantize(std::vector<std::vector<float>>& band, float qstep) {
    for (auto& row : band)
        for (auto& v : row)
            v = std::round(v / qstep);
}

// Dequantize a subband in-place
void dequantize(std::vector<std::vector<float>>& band, float qstep) {
    for (auto& row : band)
        for (auto& v : row)
            v = v * qstep;
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

    if (R.size() != (size_t)rows || G.size() != (size_t)rows || B.size() != (size_t)rows ||
        R[0].size() != (size_t)cols || G[0].size() != (size_t)cols || B[0].size() != (size_t)cols) {
        std::cerr << "❌ Loaded images have inconsistent sizes." << std::endl;
        return -1;
    }

    std::vector<std::vector<std::vector<float>>> channels = {R, G, B};
    std::vector<std::vector<std::vector<float>>> channels_reconstructed;

    // --- Adaptive Quantization: set different qsteps for each subband ---
    float q_LL2  = 3.0f;   // Most important, lowest quantization
    float q_LH2  = 6.0f;
    float q_HL2  = 6.0f;
    float q_HH2  = 12.0f;  // Least important, highest quantization
    float q_LH1  = 6.0f;
    float q_HL1  = 6.0f;
    float q_HH1  = 12.0f;

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

        // --- Adaptive Quantization ---
        quantize(LL2, q_LL2);
        quantize(LH2, q_LH2);
        quantize(HL2, q_HL2);
        quantize(HH2, q_HH2);
        quantize(LH1, q_LH1);
        quantize(HL1, q_HL1);
        quantize(HH1, q_HH1);

        // --- Flatten and concatenate all subbands (LL2, LH2, HL2, HH2, LH1, HL1, HH1) ---
        std::vector<int> flat_LL2 = flatten(LL2);
        std::vector<int> flat_LH2 = flatten(LH2);
        std::vector<int> flat_HL2 = flatten(HL2);
        std::vector<int> flat_HH2 = flatten(HH2);
        std::vector<int> flat_LH1 = flatten(LH1);
        std::vector<int> flat_HL1 = flatten(HL1);
        std::vector<int> flat_HH1 = flatten(HH1);

        // Store sizes for splitting during decoding
        size_t sz_LL2 = flat_LL2.size();
        size_t sz_LH2 = flat_LH2.size();
        size_t sz_HL2 = flat_HL2.size();
        size_t sz_HH2 = flat_HH2.size();
        size_t sz_LH1 = flat_LH1.size();
        size_t sz_HL1 = flat_HL1.size();
        size_t sz_HH1 = flat_HH1.size();

        // Concatenate all
        std::vector<int> flat_all;
        flat_all.reserve(sz_LL2 + sz_LH2 + sz_HL2 + sz_HH2 + sz_LH1 + sz_HL1 + sz_HH1);
        flat_all.insert(flat_all.end(), flat_LL2.begin(), flat_LL2.end());
        flat_all.insert(flat_all.end(), flat_LH2.begin(), flat_LH2.end());
        flat_all.insert(flat_all.end(), flat_HL2.begin(), flat_HL2.end());
        flat_all.insert(flat_all.end(), flat_HH2.begin(), flat_HH2.end());
        flat_all.insert(flat_all.end(), flat_LH1.begin(), flat_LH1.end());
        flat_all.insert(flat_all.end(), flat_HL1.begin(), flat_HL1.end());
        flat_all.insert(flat_all.end(), flat_HH1.begin(), flat_HH1.end());

        // --- Huffman encode ---
        std::unordered_map<int, std::string> huffTable;
        std::string encoded = huffmanEncode(flat_all, huffTable);

        // --- Huffman decode ---
        std::unordered_map<std::string, int> reverseTable;
        for (const auto& [val, code] : huffTable) reverseTable[code] = val;
        std::vector<int> decoded = huffmanDecode(encoded, reverseTable, flat_all.size());

        // --- Split decoded data back into subbands ---
        std::vector<int>::const_iterator it = decoded.begin();
        std::vector<std::vector<float>> rec_LL2 = unflatten(std::vector<int>(it, it + sz_LL2), LL2.size(), LL2[0].size()); it += sz_LL2;
        std::vector<std::vector<float>> rec_LH2 = unflatten(std::vector<int>(it, it + sz_LH2), LH2.size(), LH2[0].size()); it += sz_LH2;
        std::vector<std::vector<float>> rec_HL2 = unflatten(std::vector<int>(it, it + sz_HL2), HL2.size(), HL2[0].size()); it += sz_HL2;
        std::vector<std::vector<float>> rec_HH2 = unflatten(std::vector<int>(it, it + sz_HH2), HH2.size(), HH2[0].size()); it += sz_HH2;
        std::vector<std::vector<float>> rec_LH1 = unflatten(std::vector<int>(it, it + sz_LH1), LH1.size(), LH1[0].size()); it += sz_LH1;
        std::vector<std::vector<float>> rec_HL1 = unflatten(std::vector<int>(it, it + sz_HL1), HL1.size(), HL1[0].size()); it += sz_HL1;
        std::vector<std::vector<float>> rec_HH1 = unflatten(std::vector<int>(it, it + sz_HH1), HH1.size(), HH1[0].size()); it += sz_HH1;

        // --- Adaptive Dequantization ---
        dequantize(rec_LL2, q_LL2);
        dequantize(rec_LH2, q_LH2);
        dequantize(rec_HL2, q_HL2);
        dequantize(rec_HH2, q_HH2);
        dequantize(rec_LH1, q_LH1);
        dequantize(rec_HL1, q_HL1);
        dequantize(rec_HH1, q_HH1);

        // --- Reconstruct using all subbands ---
        std::vector<std::vector<float>> reconstructed_LL1 = idwt2D_db4(
            rec_LL2, rec_LH2, rec_HL2, rec_HH2);

        std::vector<std::vector<float>> reconstructed = idwt2D_db4(
            reconstructed_LL1, rec_LH1, rec_HL1, rec_HH1);

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

        // --- Normalize both images to [0,255] for fair evaluation ---
        auto normalize = [](std::vector<std::vector<float>>& img) {
            float minVal = img[0][0], maxVal = img[0][0];
            for (const auto& row : img)
                for (float v : row) {
                    if (v < minVal) minVal = v;
                    if (v > maxVal) maxVal = v;
                }
            if (maxVal > minVal) {
                for (auto& row : img)
                    for (float& v : row)
                        v = 255.0f * (v - minVal) / (maxVal - minVal);
            }
        };
        normalize(image);
        normalize(reconstructed);

        std::cout << "[5] Evaluating..." << std::endl;
        evaluate(image, reconstructed);
        double ssim = computeSSIM(image, reconstructed);
        std::cout << "SSIM: " << ssim << std::endl;

        double originalSize = static_cast<double>(flat_all.size()) * sizeof(int);
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

    // --- Compute and print mean SAM ---
    double meanSAM = computeMeanSAM(channels, channels_reconstructed);
    std::cout << "Mean SAM (radians): " << meanSAM << std::endl;
    std::cout << "Mean SAM (degrees): " << (meanSAM * 180.0 / M_PI) << std::endl;

    return 0;
}