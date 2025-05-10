#include "image_io.hpp"
#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

std::vector<std::vector<float>> loadImage(const std::string& path) {
    cv::Mat img = cv::imread(path, cv::IMREAD_GRAYSCALE);
    std::vector<std::vector<float>> result(img.rows, std::vector<float>(img.cols));
    for (int i = 0; i < img.rows; ++i)
        for (int j = 0; j < img.cols; ++j)
            result[i][j] = img.at<uchar>(i, j);
    return result;
}

void saveImage(const std::vector<std::vector<float>>& image, const std::string& path) {
    int h = image.size();
    int w = image[0].size();
    cv::Mat img(h, w, CV_8UC1);
    for (int i = 0; i < h; ++i)
        for (int j = 0; j < w; ++j)
            img.at<uchar>(i, j) = static_cast<uchar>(std::min(std::max(image[i][j], 0.0f), 255.0f));
    cv::imwrite(path, img);
}

void saveColorImage(const std::vector<std::vector<float>>& R,
                    const std::vector<std::vector<float>>& G,
                    const std::vector<std::vector<float>>& B,
                    const std::string& path) {
    int h = R.size();
    int w = R[0].size();
    cv::Mat colorImg(h, w, CV_8UC3);
    for (int i = 0; i < h; ++i)
        for (int j = 0; j < w; ++j)
            colorImg.at<cv::Vec3b>(i, j) = cv::Vec3b(
                std::clamp((int)B[i][j], 0, 255),
                std::clamp((int)G[i][j], 0, 255),
                std::clamp((int)R[i][j], 0, 255)
            );
    cv::imwrite(path, colorImg);
}
