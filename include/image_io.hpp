#pragma once
#include <vector>
#include <string>

// Loads a binary float image (square or rectangular)
std::vector<std::vector<float>> loadBinImage(const std::string& path, int& rows, int& cols);

// Saves a single-channel float image as PNG/JPG (auto-clamps to [0,255])
void saveImage(const std::vector<std::vector<float>>& image, const std::string& path);

// Saves a 3-channel float image as PNG/JPG (auto-clamps to [0,255])
void saveColorImage(const std::vector<std::vector<float>>& R,
                    const std::vector<std::vector<float>>& G,
                    const std::vector<std::vector<float>>& B,
                    const std::string& path);