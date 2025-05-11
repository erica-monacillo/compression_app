#include <iostream>
#include <fstream>
#include "dwt.hpp"
#include "huffman.hpp"
#include "image_io.hpp"
#include "utils.hpp"

int main() {
    std::string outputPath = "output/reconstructed_image.png";
    int rows = 145, cols = 145;

    std::cout << "[1] Loading raw hyperspectral bands..." << std::endl;
    std::vector<std::vector<float>> R = loadBinImage("data/band_0.bin", rows, cols);
    std::vector<std::vector<float>> G = loadBinImage("data/band_1.bin", rows, cols);
    std::vector<std::vector<float>> B = loadBinImage("data/band_2.bin", rows, cols);

    std::vector<std::vector<std::vector<float>>> channels = {R, G, B};
    std::vector<std::vector<std::vector<float>>> channels_reconstructed;

    for (int c = 0; c < 3; ++c) {
        std::cout << "\n=== Processing Channel " << c << " ===" << std::endl;
        auto image = channels[c];

        // Padding for even size
        if (image.size() % 2 != 0)
            image.push_back(image.back());
        if (image[0].size() % 2 != 0)
            for (auto& row : image)
                row.push_back(row.back());

        std::cout << "[2] Applying Level 1 DWT..." << std::endl;
        std::vector<std::vector<float>> LL1, LH1, HL1, HH1;
        dwt2D(image, LL1, LH1, HL1, HH1);

        // Padding LL1 for Level 2
        if (LL1.size() % 2 != 0)
            LL1.push_back(LL1.back());
        if (LL1[0].size() % 2 != 0)
            for (auto& row : LL1)
                row.push_back(row.back());

        std::cout << "[2] Applying Level 2 DWT..." << std::endl;
        std::vector<std::vector<float>> LL2, LH2, HL2, HH2;
        dwt2D(LL1, LL2, LH2, HL2, HH2);

        // Crash-safe check
        if (LL2.empty() || LL2[0].empty() || LH2.empty() || LH2[0].empty() || HL2.empty() || HL2[0].empty() || HH2.empty() || HH2[0].empty()) {
            std::cerr << "❌ Error: One or more Level-2 DWT sub-bands are empty!" << std::endl;
            return -1;
        }

        std::cout << "[3] Flattening + Huffman Encoding..." << std::endl;
        std::vector<int> flattened = flatten(LL2);
        std::string encoded = huffmanEncode(flattened);
        std::vector<int> decoded = huffmanDecode(encoded);

        size_t expectedSize = LL2.size() * LL2[0].size();
        if (decoded.size() < expectedSize) {
            std::cerr << "❌ Error: Decoded Huffman data is smaller than expected (" 
                      << decoded.size() << " < " << expectedSize << ")." << std::endl;
            return -1;
        }

        std::vector<std::vector<float>> reconstructed_LL2 = unflatten(decoded, LL2.size(), LL2[0].size());

        std::cout << "[4] Reconstructing..." << std::endl;

        // IDWT 1: LL2 → LL1
        std::vector<std::vector<float>> LH2_z(LH2.size(), std::vector<float>(LH2[0].size(), 0.0f));
        std::vector<std::vector<float>> HL2_z(HL2.size(), std::vector<float>(HL2[0].size(), 0.0f));
        std::vector<std::vector<float>> HH2_z(HH2.size(), std::vector<float>(HH2[0].size(), 0.0f));
        std::vector<std::vector<float>> reconstructed_LL1 = idwt2D(reconstructed_LL2, LH2_z, HL2_z, HH2_z);

        // Crash-safe check for level-1 IDWT
        if (LH1.empty() || HL1.empty() || HH1.empty()) {
            std::cerr << "❌ Error: LH1, HL1, or HH1 is empty after Level 1 DWT." << std::endl;
            return -1;
        }
        if (LH1[0].empty() || HL1[0].empty() || HH1[0].empty()) {
            std::cerr << "❌ Error: LH1[0], HL1[0], or HH1[0] is empty." << std::endl;
            std::cerr << "Sizes: LH1=" << LH1.size() << "x" << (LH1.empty() ? 0 : LH1[0].size())
                    << ", HL1=" << HL1.size() << "x" << (HL1.empty() ? 0 : HL1[0].size())
                    << ", HH1=" << HH1.size() << "x" << (HH1.empty() ? 0 : HH1[0].size()) << std::endl;
            return -1;
        }


        // IDWT 2: LL1 → reconstructed image
        std::vector<std::vector<float>> LH1_z(LH1.size(), std::vector<float>(LH1[0].size(), 0.0f));
        std::vector<std::vector<float>> HL1_z(HL1.size(), std::vector<float>(HL1[0].size(), 0.0f));
        std::vector<std::vector<float>> HH1_z(HH1.size(), std::vector<float>(HH1[0].size(), 0.0f));
        std::vector<std::vector<float>> reconstructed = idwt2D(reconstructed_LL1, LH1_z, HL1_z, HH1_z);

        std::cout << "[5] Evaluating..." << std::endl;
        evaluate(image, reconstructed);
        double ssim = computeSSIM(image, reconstructed);
        std::cout << "SSIM: " << ssim << std::endl;

        double originalSize = flattened.size() * sizeof(int);
        double compressedSize = encoded.size() / 8.0;
        double cr = originalSize / compressedSize;
        double bpp = (compressedSize * 8.0) / (image.size() * image[0].size());

        std::cout << "Compression Ratio (CR): " << cr << std::endl;
        std::cout << "Bits Per Pixel (BPP): " << bpp << std::endl;

        std::string binFilename = "output/encoded_band_" + std::to_string(c) + ".bin";
        std::ofstream out(binFilename, std::ios::binary);
        out.write(encoded.c_str(), encoded.size());
        out.close();

        channels_reconstructed.push_back(reconstructed);
    }

    std::cout << "[6] Saving full color output..." << std::endl;
    saveColorImage(channels_reconstructed[0], channels_reconstructed[1], channels_reconstructed[2], outputPath);

    std::cout << "✅ DONE! Output saved to " << outputPath << std::endl;
    return 0;
}
