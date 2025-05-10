#pragma once
#include <string>
#include <vector>

std::vector<std::vector<float>> loadImage(const std::string& path);
void saveImage(const std::vector<std::vector<float>>& image, const std::string& path);

// Optional: for loading/saving 3-band RGB images
void saveColorImage(const std::vector<std::vector<float>>& R,
                    const std::vector<std::vector<float>>& G,
                    const std::vector<std::vector<float>>& B,
                    const std::string& path);