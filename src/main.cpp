#include <iostream>
#include <fstream>
#include <unordered_map>
#include <sstream>
#include "dwt_db4.hpp"
#include "huffman.hpp"
#include "image_io.hpp"
#include "utils.hpp"

int main() {
    std::string outputPath = "output/reconstructed_image.png";
    int rows = 145;
    int cols = 145;

    // Declare a container to store multiple bands
    std::vector<std::vector<std::vector<float>>> bands;

    std::cout << "[1] Loading raw hyperspectral bands..." << std::endl;
    // Load bands 0 to 199
    for (int i = 0; i <= 199; ++i) {
        std::ostringstream filename;
        filename << "data/band_" << i << ".bin";
        std::vector<std::vector<float>> band = loadBinImage(filename.str(), rows, cols);
        bands.push_back(band);
        std::cout << "Loaded band_" << i << ".bin\n";
    }

    // Create a container for reconstructed channels
    std::vector<std::vector<std::vector<float>>> channels_reconstructed;

    // Process only the first 3 channels as RGB (you can extend this as needed)
    for (int c = 0; c < 3; ++c) {
        std::cout << "\n=== Processing Channel " << c << " ===" << std::endl;
        auto image = bands[c];

        int originalRows = image.size();
        int originalCols = image[0].size();

        // Pad to even size
        if (originalRows % 2 != 0) image.push_back(image.back());
        if (originalCols % 2 != 0) for (auto& row : image) row.push_back(row.back());

        std::cout << "[2] Applying Level 1 DWT (db4)..." << std::endl;
        std::vector<std::vector<float>> LL1, LH1, HL1, HH1;
        dwt2D_db4(image, LL1, LH1, HL1, HH1);

        if (LL1.size() % 2 != 0) LL1.push_back(LL1.back());
        if (LL1[0].size() % 2 != 0) for (auto& row : LL1) row.push_back(row.back());

        std::cout << "[2] Applying Level 2 DWT (db4)..." << std::endl;
        std::vector<std::vector<float>> LL2, LH2, HL2, HH2;
        dwt2D_db4(LL1, LL2, LH2, HL2, HH2);

        std::cout << "[3] Flattening + Real Huffman Encoding..." << std::endl;
        std::vector<int> flattened = flatten(LL2);
        std::unordered_map<int, std::string> huffTable;
        std::string encoded = huffmanEncode(flattened, huffTable);

        std::unordered_map<std::string, int> reverseTable;
        for (const auto& pair : huffTable)
            reverseTable[pair.second] = pair.first;

        std::vector<int> decoded = huffmanDecode(encoded, reverseTable);

        size_t expectedSize = LL2.size() * LL2[0].size();
        if (decoded.size() < expectedSize) {
            std::cerr << "❌ Error: Decoded size too small!" << std::endl;
            return -1;
        }

        std::vector<std::vector<float>> reconstructed_LL2 = unflatten(decoded, originalRows / 4, originalCols / 4);

        std::cout << "[4] Reconstructing..." << std::endl;
        auto zeroBand = [](int h, int w) {
            return std::vector<std::vector<float>>(h, std::vector<float>(w, 0.0f));
        };
        auto LH2_z = zeroBand(LH2.size(), LH2[0].size());
        auto HL2_z = zeroBand(HL2.size(), HL2[0].size());
        auto HH2_z = zeroBand(HH2.size(), HH2[0].size());
        std::vector<std::vector<float>> reconstructed_LL1 = idwt2D_db4(reconstructed_LL2, LH2_z, HL2_z, HH2_z);

        auto LH1_z = zeroBand(LH1.size(), LH1[0].size());
        auto HL1_z = zeroBand(HL1.size(), HL1[0].size());
        auto HH1_z = zeroBand(HH1.size(), HH1[0].size());

        std::vector<std::vector<float>> reconstructed = idwt2D_db4(reconstructed_LL1, LH1_z, HL1_z, HH1_z);

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
