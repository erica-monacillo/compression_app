#include <iostream>
#include "dwt.hpp"
#include "huffman.hpp"
#include "image_io.hpp"
#include "utils.hpp"

int main() {
    std::string inputPath = "data/sample_band.png";
    std::string outputPath = "output/reconstructed.png";

    std::vector<std::vector<float>> image = loadImage(inputPath);

    std::vector<std::vector<float>> LL, LH, HL, HH;
    dwt2D(image, LL, LH, HL, HH);

    std::vector<int> flattened = flatten(LL);
    std::string encoded = huffmanEncode(flattened);
    std::vector<int> decoded = huffmanDecode(encoded);

    std::vector<std::vector<float>> reconstructed_LL = unflatten(decoded, LL.size(), LL[0].size());
    std::vector<std::vector<float>> reconstructed = idwt2D(reconstructed_LL, LH, HL, HH);

    saveImage(reconstructed, outputPath);

    evaluate(image, reconstructed);
    return 0;
}
