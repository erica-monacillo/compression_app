#include <iostream>
#include "dwt.hpp"
#include "huffman.hpp"
#include "image_io.hpp"
#include "utils.hpp"

int main() {
    std::string inputPath = "compression_app/data/sample-band.png";
    std::string outputPath = "output/reconstructed.png";

    std::cout << "[1] Loading image..." << std::endl;
    std::vector<std::vector<float>> image = loadImage(inputPath);

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

    std::cout << "[5] Saving output image..." << std::endl;
    saveImage(reconstructed, outputPath);

    std::cout << "[6] Evaluating results..." << std::endl;
    evaluate(image, reconstructed);

    std::cout << "✅ DONE! Check output/reconstructed.png" << std::endl;
    return 0;
}