#pragma once
#include <string>
#include <vector>

std::vector<std::vector<float>> loadBinImage(const std::string& path, int rows, int cols);
void saveImage(const std::vector<std::vector<float>>& image, const std::string& path);
void saveColorImage(const std::vector<std::vector<float>>& R,
                    const std::vector<std::vector<float>>& G,
                    const std::vector<std::vector<float>>& B,
                    const std::string& path);
