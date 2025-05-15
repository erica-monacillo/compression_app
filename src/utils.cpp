#include "utils.hpp"
#include <iostream>
#include <cmath>
#include <algorithm>

// Flatten 2D float matrix into 1D integer vector
std::vector<int> flatten(const std::vector<std::vector<float>>& mat) {
    std::vector<int> result;
    for (const auto& row : mat)
        for (float val : row)
            result.push_back(static_cast<int>(std::round(val))); // rounded
    return result;
}

// Unflatten 1D int vector into 2D float matrix (rows × cols)
std::vector<std::vector<float>> unflatten(const std::vector<int>& vec, int rows, int cols) {
    std::vector<std::vector<float>> mat(rows, std::vector<float>(cols));
    size_t idx = 0;
    for (int i = 0; i < rows && idx < vec.size(); ++i)
        for (int j = 0; j < cols && idx < vec.size(); ++j)
            mat[i][j] = static_cast<float>(vec[idx++]);
    return mat;
}

// Compute MSE and PSNR between original and reconstructed image
void evaluate(const std::vector<std::vector<float>>& orig, const std::vector<std::vector<float>>& recon) {
    double mse = 0.0;
    int h = orig.size(), w = orig[0].size();

    for (int i = 0; i < h; ++i)
        for (int j = 0; j < w; ++j)
            mse += std::pow(orig[i][j] - recon[i][j], 2);

    mse /= (h * w);
    double psnr = 10.0 * std::log10((255.0 * 255.0) / (mse + 1e-8)); // add epsilon to avoid divide by zero
    std::cout << "MSE: " << mse << "\nPSNR: " << psnr << " dB" << std::endl;
}

// Compute Structural Similarity Index (SSIM)
double computeSSIM(const std::vector<std::vector<float>>& img1,
                   const std::vector<std::vector<float>>& img2) {
    int h = img1.size(), w = img1[0].size();
    double mu1 = 0.0, mu2 = 0.0;

    for (int i = 0; i < h; ++i)
        for (int j = 0; j < w; ++j) {
            mu1 += img1[i][j];
            mu2 += img2[i][j];
        }
    mu1 /= (h * w);
    mu2 /= (h * w);

    double sigma1_sq = 0.0, sigma2_sq = 0.0, sigma12 = 0.0;
    for (int i = 0; i < h; ++i)
        for (int j = 0; j < w; ++j) {
            sigma1_sq += std::pow(img1[i][j] - mu1, 2);
            sigma2_sq += std::pow(img2[i][j] - mu2, 2);
            sigma12   += (img1[i][j] - mu1) * (img2[i][j] - mu2);
        }

    sigma1_sq /= (h * w);
    sigma2_sq /= (h * w);
    sigma12   /= (h * w);

    const double C1 = 6.5025;
    const double C2 = 58.5225;

    double numerator   = (2 * mu1 * mu2 + C1) * (2 * sigma12 + C2);
    double denominator = (mu1 * mu1 + mu2 * mu2 + C1) * (sigma1_sq + sigma2_sq + C2);
    return numerator / (denominator + 1e-8); // avoid divide by 0
}
