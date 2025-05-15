#include "image_io.hpp"
#include <fstream>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <algorithm>

std::vector<std::vector<float>> loadBinImage(const std::string& path, int rows, int cols) {
    std::vector<std::vector<float>> image(rows, std::vector<float>(cols));
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "❌ Cannot open binary file: " << path << std::endl;
        return image;
    }

    for (int i = 0; i < rows; ++i) {
        file.read(reinterpret_cast<char*>(image[i].data()), cols * sizeof(float));
        if (!file) {
            std::cerr << "❌ Error reading row " << i << " from " << path << std::endl;
            break;
        }
    }

    file.close();
    return image;
}

void saveImage(const std::vector<std::vector<float>>& image, const std::string& path) {
    int h = image.size();
    int w = image[0].size();
    cv::Mat img(h, w, CV_8UC1);
    for (int i = 0; i < h; ++i)
        for (int j = 0; j < w; ++j)
            img.at<uchar>(i, j) = static_cast<uchar>(std::clamp(image[i][j], 0.0f, 255.0f));
    if (!cv::imwrite(path, img)) {
        std::cerr << "❌ Failed to write image to " << path << std::endl;
    }
}

void saveColorImage(const std::vector<std::vector<float>>& R,
                    const std::vector<std::vector<float>>& G,
                    const std::vector<std::vector<float>>& B,
                    const std::string& path) {
    int h = R.size();
    int w = R[0].size();
    cv::Mat colorImg(h, w, CV_8UC3);
    for (int i = 0; i < h; ++i) {
        for (int j = 0; j < w; ++j) {
            int r = static_cast<int>(std::round(R[i][j]));
            int g = static_cast<int>(std::round(G[i][j]));
            int b = static_cast<int>(std::round(B[i][j]));

            colorImg.at<cv::Vec3b>(i, j) = cv::Vec3b(
                std::clamp(b, 0, 255),
                std::clamp(g, 0, 255),
                std::clamp(r, 0, 255)
            );
        }
    }
    if (!cv::imwrite(path, colorImg)) {
        std::cerr << "❌ Failed to save color image to " << path << std::endl;
    }
}
