#include <iostream>
#include <fstream>
#include <unordered_map>
#include "dwt_db4.hpp"
#include "huffman.hpp"
#include "image_io.hpp"
#include "utils.hpp"
#include <opencv2/opencv.hpp>

int main() {
    std::string outputPath = "output/reconstructed_image.png";
    int total_values = 20750; // from above
    int side = static_cast<int>(std::sqrt(total_values)); // ≈ 144

    int rows = 144;
    int cols = 144;
    int originalRows = rows;
    int originalCols = cols;



    std::cout << "[1] Loading raw hyperspectral bands..." << std::endl;
    std::vector<std::vector<float>> R = loadBinImage("data/band_0.bin", rows, cols);
    std::vector<std::vector<float>> G = loadBinImage("data/band_1.bin", rows, cols);
    std::vector<std::vector<float>> B = loadBinImage("data/band_2.bin", rows, cols);
    std::vector<std::vector<std::vector<float>>> channels = {R, G, B};
    std::vector<std::vector<std::vector<float>>> channels_reconstructed;

    for (int c = 0; c < 3; ++c) {
        std::cout << "\n=== Processing Channel " << c << " ===" << std::endl;
        auto image = channels[c];

        // Pad to even size
        if (image.size() % 2 != 0) image.push_back(image.back());
        if (image[0].size() % 2 != 0) for (auto& row : image) row.push_back(row.back());

        std::cout << "[2] Applying Level 1 DWT (db4)..." << std::endl;
        std::vector<std::vector<float>> LL1, LH1, HL1, HH1;
        dwt2D_db4(image, LL1, LH1, HL1, HH1);

        if (LL1.size() % 2 != 0) LL1.push_back(LL1.back());
        if (LL1[0].size() % 2 != 0) for (auto& row : LL1) row.push_back(row.back());

        std::cout << "[2] Applying Level 2 DWT (db4)..." << std::endl;
        std::vector<std::vector<float>> LL2, LH2, HL2, HH2;
        dwt2D_db4(LL1, LL2, LH2, HL2, HH2);

        std::cout << "[3] Flattening + Real Huffman Encoding..." << std::endl;
        if (LL2.size() % 2 != 0) LL2.push_back(LL2.back());
        if (LL2[0].size() % 2 != 0) for (auto& row : LL2) row.push_back(row.back());

        std::vector<int> flattened = flatten(LL2);
        std::unordered_map<int, std::string> huffTable;
        std::string encoded = huffmanEncode(flattened, huffTable);

        std::unordered_map<std::string, int> reverseTable;
        for (auto& pair : huffTable)
            reverseTable[pair.second] = pair.first;

        std::vector<int> decoded = huffmanDecode(encoded, reverseTable);

        int ll2_rows = originalRows / 4;
        int ll2_cols = originalCols / 4;
        size_t expectedSize = ll2_rows * ll2_cols;

        if (decoded.size() != expectedSize) {
            std::cerr << "❌ Error: Decoded size (" << decoded.size() 
                    << ") does not match expected size (" << expectedSize << ")!" << std::endl;
            return -1;
        }


        std::cout << "Decoded size: " << decoded.size() << std::endl;
        std::cout << "Expected size (LL2): " << ll2_rows << " x " << ll2_cols
                << " = " << expectedSize << std::endl;

        std::vector<std::vector<float>> reconstructed_LL2;
        try {
            std::cout << "Unflattening to " << ll2_rows << " x " << ll2_cols << std::endl;
            std::cout << "Decoded vector size: " << decoded.size() << std::endl;

            reconstructed_LL2 = unflatten(decoded, ll2_rows, ll2_cols);
        } catch (const std::exception& e) {
            std::cerr << "❌ Exception in unflatten: " << e.what() << std::endl;
            return -1;
        }

        std::cout << "[4] Reconstructing..." << std::endl;

        // Zero-band generator
        auto zeroBand = [](int h, int w) {
            return std::vector<std::vector<float>>(h, std::vector<float>(w, 0.0f));
        };

        // Pad reconstructed_LL1 to even size before inverse DWT Level 1
        if (reconstructed_LL1.size() % 2 != 0) reconstructed_LL1.push_back(reconstructed_LL1.back());
        if (reconstructed_LL1[0].size() % 2 != 0)
            for (auto& row : reconstructed_LL1) row.push_back(row.back());

        // Recompute dimensions after padding
        int ll1_rows = reconstructed_LL1.size();
        int ll1_cols = reconstructed_LL1[0].size();

        if (ll2_rows == 0 || ll2_cols == 0) {
            std::cerr << "❌ Error: reconstructed_LL2 is empty or invalid size!" << std::endl;
            return -1;
        }

        // Now make zero bands that match
        auto LH1_z = zeroBand(ll1_rows, ll1_cols);
        auto HL1_z = zeroBand(ll1_rows, ll1_cols);
        auto HH1_z = zeroBand(ll1_rows, ll1_cols);

        // Pad reconstructed_LL2 to even size before inverse DWT Level 2
        if (reconstructed_LL2.size() % 2 != 0) reconstructed_LL2.push_back(reconstructed_LL2.back());
        if (reconstructed_LL2[0].size() % 2 != 0)
            for (auto& row : reconstructed_LL2) row.push_back(row.back());

        // Pad zero bands as well (if needed)
        if (LH2_z.size() % 2 != 0) LH2_z.push_back(LH2_z.back());
        if (LH2_z[0].size() % 2 != 0)
            for (auto& row : LH2_z) row.push_back(row.back());

        if (HL2_z.size() % 2 != 0) HL2_z.push_back(HL2_z.back());
        if (HL2_z[0].size() % 2 != 0)
            for (auto& row : HL2_z) row.push_back(row.back());

        if (HH2_z.size() % 2 != 0) HH2_z.push_back(HH2_z.back());
        if (HH2_z[0].size() % 2 != 0)
            for (auto& row : HH2_z) row.push_back(row.back());


        // Inverse DWT Level 2
        std::vector<std::vector<float>> reconstructed = idwt2D_db4(reconstructed_LL1, LH1_z, HL1_z, HH1_z);

        // Level 1 zero bands
        int ll1_rows = reconstructed_LL1.size();
        int ll1_cols = reconstructed_LL1.empty() ? 0 : reconstructed_LL1[0].size();

        auto LH1_z = zeroBand(ll1_rows, ll1_cols);
        auto HL1_z = zeroBand(ll1_rows, ll1_cols);
        auto HH1_z = zeroBand(ll1_rows, ll1_cols);

        // Inverse DWT Level 1
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
