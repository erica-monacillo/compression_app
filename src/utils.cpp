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

// Compute mean Spectral Angle Mapper (SAM) between original and reconstructed images
double computeMeanSAM(
    const std::vector<std::vector<std::vector<float>>>& original,
    const std::vector<std::vector<std::vector<float>>>& reconstructed)
{
    int bands = original.size();
    int rows = original[0].size();
    int cols = original[0][0].size();
    double sam_sum = 0.0;
    int count = 0;

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            // Build spectrum vectors for this pixel
            std::vector<float> orig_spec(bands), recon_spec(bands);
            for (int b = 0; b < bands; ++b) {
                orig_spec[b] = original[b][i][j];
                recon_spec[b] = reconstructed[b][i][j];
            }
            // Compute dot product and norms
            double dot = 0.0, norm_orig = 0.0, norm_recon = 0.0;
            for (int b = 0; b < bands; ++b) {
                dot += orig_spec[b] * recon_spec[b];
                norm_orig += orig_spec[b] * orig_spec[b];
                norm_recon += recon_spec[b] * recon_spec[b];
            }
            norm_orig = std::sqrt(norm_orig);
            norm_recon = std::sqrt(norm_recon);
            if (norm_orig > 0 && norm_recon > 0) {
                double cos_theta = dot / (norm_orig * norm_recon);
                // Clamp to [-1, 1] to avoid NaNs
                cos_theta = std::max(-1.0, std::min(1.0, cos_theta));
                double angle = std::acos(cos_theta); // in radians
                sam_sum += angle;
                ++count;
            }
        }
    }
    return (count > 0) ? (sam_sum / count) : 0.0; // mean SAM in radians
}