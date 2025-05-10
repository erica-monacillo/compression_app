#pragma once
#include <vector>

std::vector<int> flatten(const std::vector<std::vector<float>>& mat);
std::vector<std::vector<float>> unflatten(const std::vector<int>& vec, int rows, int cols);
void evaluate(const std::vector<std::vector<float>>& orig, const std::vector<std::vector<float>>& recon);

double computeSSIM(const std::vector<std::vector<float>>& img1,
                   const std::vector<std::vector<float>>& img2);
