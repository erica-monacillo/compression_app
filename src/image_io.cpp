#include "image_io.hpp"
#include <fstream>
#include <iostream>
#include <opencv2/opencv.hpp>
<<<<<<< Updated upstream

std::vector<std::vector<float>> loadBinImage(const std::string& path, int rows, int cols) {
    std::vector<std::vector<float>> image(rows, std::vector<float>(cols));
    std::ifstream file(path, std::ios::binary);
=======
#include <algorithm>
#include <cmath>

// Loads a binary float image (square or rectangular)
std::vector<std::vector<float>> loadBinImage(const std::string& path, int& rows, int& cols) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
>>>>>>> Stashed changes
    if (!file) {
        std::cerr << "❌ Cannot open binary file: " << path << std::endl;
        rows = cols = 0;
        return {};
    }

<<<<<<< Updated upstream
    for (int i = 0; i < rows; ++i)
        file.read(reinterpret_cast<char*>(image[i].data()), cols * sizeof(float));
=======
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    int total_floats = static_cast<int>(size / sizeof(float));
    if (rows <= 0 || cols <= 0) {
        // Try to infer square image if not provided
        int dim = static_cast<int>(std::sqrt(total_floats));
        if (dim * dim != total_floats) {
            std::cerr << "❌ File size does not match a square image for " << path << std::endl;
            rows = cols = 0;
            return {};
        }
        rows = cols = dim;
    }
    if (rows * cols != total_floats) {
        std::cerr << "❌ File size does not match given dimensions for " << path << std::endl;
        return {};
    }
>>>>>>> Stashed changes

    std::vector<float> buffer(total_floats);
    file.read(reinterpret_cast<char*>(buffer.data()), total_floats * sizeof(float));
    if (!file) {
        std::cerr << "❌ Failed to read all data from " << path << std::endl;
        return {};
    }

    std::vector<std::vector<float>> image(rows, std::vector<float>(cols));
    for (int i = 0; i < rows; ++i)
        std::copy(buffer.begin() + i * cols, buffer.begin() + (i + 1) * cols, image[i].begin());

    return image;
}

// Saves a single-channel float image as PNG/JPG (auto-clamps to [0,255])
void saveImage(const std::vector<std::vector<float>>& image, const std::string& path) {
    if (image.empty() || image[0].empty()) {
        std::cerr << "❌ Empty image, cannot save to " << path << std::endl;
        return;
    }
    int h = image.size();
    int w = image[0].size();
    cv::Mat img(h, w, CV_8UC1);
    for (int i = 0; i < h; ++i)
        for (int j = 0; j < w; ++j)
<<<<<<< Updated upstream
            img.at<uchar>(i, j) = static_cast<uchar>(std::clamp(image[i][j], 0.0f, 255.0f));
    cv::imwrite(path, img);
=======
            img.at<uchar>(i, j) = static_cast<uchar>(std::clamp(std::round(image[i][j]), 0.f, 255.f));
    if (!cv::imwrite(path, img)) {
        std::cerr << "❌ Failed to write image to " << path << std::endl;
    }
>>>>>>> Stashed changes
}

// Saves a 3-channel float image as PNG/JPG (auto-clamps to [0,255])
void saveColorImage(const std::vector<std::vector<float>>& R,
                    const std::vector<std::vector<float>>& G,
                    const std::vector<std::vector<float>>& B,
                    const std::string& path) {
    if (R.empty() || G.empty() || B.empty() ||
        R.size() != G.size() || R.size() != B.size() ||
        R[0].size() != G[0].size() || R[0].size() != B[0].size()) {
        std::cerr << "❌ Channel size mismatch or empty, cannot save color image to " << path << std::endl;
        return;
    }
    int h = R.size();
    int w = R[0].size();
    cv::Mat colorImg(h, w, CV_8UC3);
<<<<<<< Updated upstream
    for (int i = 0; i < h; ++i)
        for (int j = 0; j < w; ++j)
=======
    for (int i = 0; i < h; ++i) {
        for (int j = 0; j < w; ++j) {
            int r = static_cast<int>(std::round(R[i][j]));
            int g = static_cast<int>(std::round(G[i][j]));
            int b = static_cast<int>(std::round(B[i][j]));
>>>>>>> Stashed changes
            colorImg.at<cv::Vec3b>(i, j) = cv::Vec3b(
                std::clamp(static_cast<int>(B[i][j]), 0, 255),
                std::clamp(static_cast<int>(G[i][j]), 0, 255),
                std::clamp(static_cast<int>(R[i][j]), 0, 255)
            );
<<<<<<< Updated upstream
    cv::imwrite(path, colorImg);
}
=======
        }
    }
    if (!cv::imwrite(path, colorImg)) {
        std::cerr << "❌ Failed to save color image to " << path << std::endl;
    }
}
>>>>>>> Stashed changes
