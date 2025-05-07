#include "utils.hpp"
#include <iostream>
#include <cmath>

std::vector<int> flatten(const std::vector<std::vector<float>>& mat) {
    std::vector<int> result;
    for (auto& row : mat)
        for (auto val : row)
            result.push_back((int)val);
    return result;
}

std::vector<std::vector<float>> unflatten(const std::vector<int>& vec, int rows, int cols) {
    std::vector<std::vector<float>> mat(rows, std::vector<float>(cols));
    int idx = 0;
    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j)
            mat[i][j] = vec[idx++];
    return mat;
}

void evaluate(const std::vector<std::vector<float>>& orig, const std::vector<std::vector<float>>& recon) {
    double mse = 0;
    int h = orig.size(), w = orig[0].size();
    for (int i = 0; i < h; ++i)
        for (int j = 0; j < w; ++j)
            mse += pow(orig[i][j] - recon[i][j], 2);
    mse /= (h * w);
    double psnr = 10 * log10(255 * 255 / mse);
    std::cout << "MSE: " << mse << "\nPSNR: " << psnr << " dB" << std::endl;
}
