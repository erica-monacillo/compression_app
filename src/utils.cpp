#include "utils.hpp"
#include <iostream>
#include <cmath>
#include <numeric>

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

double computeSSIM(const std::vector<std::vector<float>>& img1,
                   const std::vector<std::vector<float>>& img2) {
    int h = img1.size(), w = img1[0].size();
    double mu1 = 0, mu2 = 0;
    for (int i = 0; i < h; ++i)
        for (int j = 0; j < w; ++j) {
            mu1 += img1[i][j];
            mu2 += img2[i][j];
        }
    mu1 /= (h * w);
    mu2 /= (h * w);

    double sigma1_sq = 0, sigma2_sq = 0, sigma12 = 0;
    for (int i = 0; i < h; ++i)
        for (int j = 0; j < w; ++j) {
            sigma1_sq += (img1[i][j] - mu1) * (img1[i][j] - mu1);
            sigma2_sq += (img2[i][j] - mu2) * (img2[i][j] - mu2);
            sigma12 += (img1[i][j] - mu1) * (img2[i][j] - mu2);
        }
    sigma1_sq /= (h * w);
    sigma2_sq /= (h * w);
    sigma12 /= (h * w);

    const double C1 = 6.5025, C2 = 58.5225;
    double ssim = ((2 * mu1 * mu2 + C1) * (2 * sigma12 + C2)) /
                  ((mu1 * mu1 + mu2 * mu2 + C1) * (sigma1_sq + sigma2_sq + C2));
    return ssim;
}
