#include <iostream>
#include <fstream>
#include <opencv2/opencv.hpp>
#include "dwt.hpp"
#include "huffman.hpp"
#include "image_io.hpp"
#include "utils.hpp"

int main() {
    std::string inputPath = "data/Indian_pines_gt.png";
    std::string outputPath = "output/reconstructed_color.png";

    std::cout << "[1] Loading RGB image..." << std::endl;
    cv::Mat inputColor = cv::imread(inputPath, cv::IMREAD_COLOR);
    if (inputColor.empty()) {
        std::cerr << "❌ Error: Cannot load image!" << std::endl;
        return -1;
    }

    std::vector<cv::Mat> bgr_channels(3);
    cv::split(inputColor, bgr_channels); // OpenCV loads as BGR

    std::vector<std::vector<std::vector<float>>> channels_reconstructed;

    for (int c = 0; c < 3; ++c) {
        std::cout << "\n=== Processing Channel " << c << " ===" << std::endl;

        std::vector<std::vector<float>> image(
            bgr_channels[c].rows,
            std::vector<float>(bgr_channels[c].cols)
        );
        for (int i = 0; i < bgr_channels[c].rows; ++i)
            for (int j = 0; j < bgr_channels[c].cols; ++j)
                image[i][j] = bgr_channels[c].at<uchar>(i, j);

        std::cout << "[2] Applying DWT..." << std::endl;
        std::vector<std::vector<float>> LL, LH, HL, HH;
        dwt2D(image, LL, LH, HL, HH);

        std::cout << "[3] Flattening + Encoding..." << std::endl;
        std::vector<int> flattened = flatten(LL);
        std::string encoded = huffmanEncode(flattened);
        std::vector<int> decoded = huffmanDecode(encoded);

        std::cout << "[4] Reconstructing..." << std::endl;
        std::vector<std::vector<float>> reconstructed_LL = unflatten(decoded, LL.size(), LL[0].size());
        std::vector<std::vector<float>> reconstructed = idwt2D(reconstructed_LL, LH, HL, HH);

        std::cout << "[5] Evaluating..." << std::endl;
        evaluate(image, reconstructed);
        double ssim = computeSSIM(image, reconstructed);
        std::cout << "SSIM: " << ssim << std::endl;

        double originalSize = flattened.size() * sizeof(int); // bytes
        double compressedSize = encoded.size() / 8.0;         // bits to bytes
        double cr = originalSize / compressedSize;
        double bpp = (compressedSize * 8.0) / (image.size() * image[0].size());

        std::cout << "Compression Ratio (CR): " << cr << std::endl;
        std::cout << "Bits Per Pixel (BPP): " << bpp << std::endl;

        // Save compressed bitstream to .bin
        std::string binFilename = "output/encoded_band_" + std::to_string(c) + ".bin";
        std::ofstream out(binFilename, std::ios::binary);
        out.write(encoded.c_str(), encoded.size());
        out.close();

        channels_reconstructed.push_back(reconstructed);
    }

    std::cout << "[6] Saving full color output..." << std::endl;
    saveColorImage(channels_reconstructed[2], channels_reconstructed[1], channels_reconstructed[0], outputPath); // RGB

    std::cout << "✅ DONE! Output saved to " << outputPath << std::endl;
    return 0;
}
