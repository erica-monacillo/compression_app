#pragma once
#include <string>
#include <vector>

std::vector<std::vector<float>> loadImage(const std::string& path);
void saveImage(const std::vector<std::vector<float>>& image, const std::string& path);
